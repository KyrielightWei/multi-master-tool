#include "json_packet.h"

/******************************************************
 * DynamicBuffer Defintion
******************************************************/
DynamicBuffer::DynamicBuffer()
{
    blocks_list.clear();
    tail_offset_in_block = 0;
}

DynamicBuffer::~DynamicBuffer()
{
    destory();
}

void DynamicBuffer::clear()
{
    for (auto it = blocks_list.begin(); it != blocks_list.end(); it++)
    {
        memset(*it, 0, BLOCK_SIZE);
    }
    tail_offset_in_block = 0;
}

void DynamicBuffer::destory()
{
    for (auto it = blocks_list.begin(); it != blocks_list.end(); it++)
    {
        delete[](*it);
    }
    blocks_list.clear();
    tail_offset_in_block = 0;
}

void DynamicBuffer::put(const char *data, int offset, int size)
{
    check_space(offset, size, true);
    bool append_byte = false;
    if(offset + size > this->size())
    {
        append_byte = true;
    }

    int index = offset / BLOCK_SIZE;

    offset = offset % BLOCK_SIZE;
    auto block_it = blocks_list.begin() + index;

    while (size != 0)
    {
        if (size > BLOCK_SIZE - offset)
        {
            memcpy((*block_it) + offset, data, BLOCK_SIZE - offset);
            size = size - (BLOCK_SIZE - offset);
            offset = 0;
            block_it++;
        }
        else
        {
            memcpy((*block_it) + offset, data, size);
        #if JSON_PACKET_DEBUG
        cout << "----------------- before put tail: " << tail_offset_in_block << " " << offset << " " << size << endl;
        #endif // 0
            if(append_byte)
                tail_offset_in_block = offset + size;
        #if JSON_PACKET_DEBUG
        cout << "----------------- after put tail: " << tail_offset_in_block<< " " << offset << " " << size << endl;
        #endif // 0
            size = 0;
        }
    }
}

char * DynamicBuffer::get_address(int offset)
{
    if(offset > size())
    {
        return NULL;
    }

    int index = offset / BLOCK_SIZE;
    offset = offset % BLOCK_SIZE;
    auto block_it = blocks_list.begin() + index;

    return (*block_it) + offset;
}

bool DynamicBuffer::get(char *data, int offset, int size)
{
    if (!check_space(offset, size, false))
    {
        return false;
    }
    int index = offset / BLOCK_SIZE;
    offset = offset % BLOCK_SIZE;
    auto block_it = blocks_list.begin() + index;

    while (size != 0)
    {
        if (size > BLOCK_SIZE - offset)
        {
            memcpy(data, (*block_it) + offset, BLOCK_SIZE - offset);
            size = size - (BLOCK_SIZE - offset);
            data = data + (BLOCK_SIZE - offset);
            offset = 0;
            block_it++;
        }
        else
        {
            memcpy(data, (*block_it) + offset, size);
            size = 0;
        }
    }
    return true;
}

int DynamicBuffer::append(const char *data, int data_size)
{
    int offset = size();
    #if JSON_PACKET_DEBUG
        cout << "************ before append_offset: " << " "<<offset << endl;
    #endif // 0
    put(data, offset, data_size);

    #if JSON_PACKET_DEBUG
        cout << "************ after append_offset: "<< size() << endl;
    #endif // 0
    return offset;
}

bool DynamicBuffer::check_space(int offset, int size, bool expand)
{
    if (offset + size > capacity())
    {
        if (!expand)
        {
            return false;
        }
        else
        {
            int require = offset + size - capacity();
            if (require % BLOCK_SIZE == 0)
            {
                require = require / BLOCK_SIZE;
            }
            else
            {
                require = require / BLOCK_SIZE + 1;
            }
            add_blocks(require);
            return true;
        }
    }
    else
    {
        return true;
    }
}

void DynamicBuffer::add_blocks(int i)
{
    // cout << "add" << i << " blocks" << endl; 
    for (int j = 0; j < i; j++)
    {
        blocks_list.push_back(new char[BLOCK_SIZE]{0});
    }
}

int DynamicBuffer::size()
{
    int count = blocks_list.size();
    if (count > 0)
    {
        count--;
    }
    return count * BLOCK_SIZE + tail_offset_in_block;
}

int DynamicBuffer::capacity()
{
    return blocks_list.size() * BLOCK_SIZE;
}

void DynamicBuffer::print_buffer()
{
    int i = 0;
    for (auto it = blocks_list.begin(); it != blocks_list.end(); it++)
    {
        std::cout << "Block NO." << i << " : " << std::endl;
        for (int j = 0; j < BLOCK_SIZE; j++)
        {
            std::cout << blocks_list[i][j];
        }
        i++;
        std::cout << std::endl;
    }
    std::cout << "Dynamic Buffer Size : " << size() << std::endl;
}

/******************************************************
 * JsonPacket 
******************************************************/
const char *JsonPacket::get_string_ptr()
{
    return json_buffer.GetString();
}

void JsonPacket::parse(const char *json_str)
{
    rapidjson::StringStream ss(json_str);
    json_reader.Parse(ss, handle);
}

void JsonPacket::clear_packet()
{
    json_buffer.Clear();
    handle.clear();
    json_writer.Reset(json_buffer);
}

void JsonPacket::set_packet_header(const char *h, int size)
{
    json_writer.StartArray();
    json_writer.StartObject();

    json_writer.Key("SegmentType");
    json_writer.Int(PacketSegmentType::HEADER);
    json_writer.Key("Header");
    json_writer.String(h, size);

    json_writer.EndObject();
}

void JsonPacket::set_packet_item(const char *key, const char *val, int val_size, PacketItemType type)
{
    if (type == PacketItemType::FIRST || type == PacketItemType::SINGLE)
    {
        json_writer.StartObject();
        json_writer.Key("SegmentType");
        json_writer.Int(PacketSegmentType::BODY);
    }

    json_writer.Key(key);
    json_writer.String(val, val_size);

    if (type == PacketItemType::LAST || type == PacketItemType::SINGLE)
    {
        json_writer.EndObject();
    }
}

void JsonPacket::set_note_for_ptr(const char *item_key, int offset, const char *val, int val_size, PacketItemType type)
{
    if (type == PacketItemType::FIRST || type == PacketItemType::SINGLE)
    {
        json_writer.StartObject();
        json_writer.Key("SegmentType");
        json_writer.Int(PacketSegmentType::PTR_NOTE);
        json_writer.Key("note_arry");
        json_writer.StartArray();
    }

    json_writer.StartObject();
    // json_writer.Key("ptr_name");
    // json_writer.String(ptr_name);
    json_writer.Key("ptr_key");
    json_writer.String(item_key);
    json_writer.Key("ptr_offset");
    json_writer.Int(offset);
    json_writer.Key("ptr_val");
    json_writer.String(val, val_size);
    json_writer.EndObject();

    if (type == PacketItemType::LAST || type == PacketItemType::SINGLE)
    {
        json_writer.EndArray();
        json_writer.EndObject();
        json_writer.EndArray();
    }
}

void JsonPacket::printdy()
{
    handle.print_buffer();
}

void JsonPacket::get_packet_header_ptr() 
{

}

void JsonPacket::get_packet_item_ptr() {}

/******************************************************
 * JsonPacket :: ReadHandler
******************************************************/
bool JsonPacket::ReadHandler::Uint(unsigned u)
{
    #if JSON_PACKET_DEBUG
        cout << "Uint(" << u << ")" << endl;// return true;
    #endif 
     
    ParseItem item;
    item = parse_stack.top();
    parse_stack.pop();

    //cout << "uint key : " << item.key << std::endl;
    if (item.key == "ptr_offset")
    {
      //  cout << "********* ptr offset : " << u << std::endl;
        ParseItem item_temp;
        item_temp.key = std::string((const char *)&u,sizeof(u));
        item_temp.type = ParseType::PTR_OFFSET;
        parse_stack.push(item_temp);
    }
    else
    {
        int offset = objectBuffer.append((const char *)&u,sizeof(u));
        object_ptrs.push_back(offset);
        name_id_map[item.key] = object_ptrs.size() - 1 ;
    }
    return true;
}

bool JsonPacket::ReadHandler::String(const char *str, SizeType length, bool copy)
{
    #if JSON_PACKET_DEBUG
       cout << "String(" << str << ", " << length << ", "  << copy << ")" << endl;
    #endif    
   
    ParseItem item;
    item  = parse_stack.top();
    parse_stack.pop();

    if(item.key == "ptr_key")
    {
        ParseItem item_temp;
        item_temp.key = std::string(str,length);
        item_temp.type = ParseType::PTR_KEY;
        parse_stack.push(item_temp);
        return true;
    }

    if(item.key == "ptr_val")
    {
        ParseItem item_key,item_offset;
        item_offset = parse_stack.top();
        parse_stack.pop();
        item_key = parse_stack.top();
        parse_stack.pop();
        unsigned ptr_offset = *((unsigned *)item_offset.key.c_str());

        //append value which ptr point to
        int val_offset = objectBuffer.append(str,length);
        const char * address = objectBuffer.get_address(val_offset);

        int index = name_id_map[item_key.key];
        int start_offset = object_ptrs[index];
        #if JSON_PACKET_DEBUG
        cout << "--------- val_offset: "<<  val_offset << endl;
        cout << "start : " << start_offset << " ptr_offset : " << ptr_offset << ";"<< std::endl;
        #endif // 0
        
        objectBuffer.put((const char *)&address,start_offset+ptr_offset,sizeof(address)); // modify address
        return true;
    }

    int offset = objectBuffer.append(str,length);
    object_ptrs.push_back(offset);
    name_id_map[item.key] = object_ptrs.size() - 1 ;
    return true;
}

bool JsonPacket::ReadHandler::StartObject()
{
     #if JSON_PACKET_DEBUG
       cout << "StartObject()" << endl;// return true;
    #endif  
    
    ParseItem item;
    item.key = "";
    item.type = ParseType::OBJECT; 
    parse_stack.push(item);
    return true;
}

bool JsonPacket::ReadHandler::Key(const char *str, SizeType length, bool copy) 
{
    #if JSON_PACKET_DEBUG
       cout << "Key(" << str << ", " << length << ", "  << copy << ")" << endl;
    #endif  
     
    ParseItem item;
    item.key = std::string(str,length);
    item.type = ParseType::KEY; 
    parse_stack.push(item);
    return true;
}

bool JsonPacket::ReadHandler::EndObject(SizeType memberCount) 
{
    #if JSON_PACKET_DEBUG
      cout << "EndObject(" << memberCount << ")" << endl; //return true;
    #endif  
   
    ParseItem item;
    item = parse_stack.top();
    parse_stack.pop();
    if(item.type != ParseType::OBJECT)
    {
        std::cout << item.type << ";" << item.key << " ERROR: Object can not find start point" << std::endl;
        return false;
    }
    return true;
}

bool JsonPacket::ReadHandler::StartArray() 
{
    #if JSON_PACKET_DEBUG
     cout << "StartArray()" << endl; //return true;
    #endif  
    
    ParseItem item;
    if(!parse_stack.empty())
        item = parse_stack.top();
    if(item.type == ParseType::KEY)
    {
        parse_stack.pop();
    }
   
    item.key = "";
    item.type = ParseType::ARRAY; 
    parse_stack.push(item);
    return true;
}

bool JsonPacket::ReadHandler::EndArray(SizeType elementCount) 
{
     #if JSON_PACKET_DEBUG
      cout << "EndArray(" << elementCount << ")" << endl;// return true;
    #endif  
    
    ParseItem item;
    item = parse_stack.top();
    parse_stack.pop();
    if(item.type != ParseType::ARRAY)
    {
        std::cout << "ERROR: Array is not empty after handing" << std::endl;
        return false;
    }
    return true;
}