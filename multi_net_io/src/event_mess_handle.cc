#include "event_mess_handle.h"

const char *MessageError::getEventErrorStr(EventMessageErrorNo no)
{
    return EVENT_ERROR_STRING[no];
}

const char *MessageError::EVENT_ERROR_STRING[] = {
    "event message has none error",
    "incomplete arg : EventMessage"

};

/***********************************
 * class  EventMessage
 * *******************************/
void EventMessage::copy(const EventMessage &src_mess)
{
    buffer_str = src_mess.buffer_str;
    buffer_size = src_mess.buffer_size;
    init_message_ptr();
}

void EventMessage::init_buffer_str()
{
    buffer_str = "";
    buffer_size = 0;

    buffer_str.append(sizeof(EventMessageCursor), 0);
    cursor = (EventMessageCursor *)buffer_str.c_str();
    buffer_size += sizeof(EventMessageCursor);

    cursor->group_name_offset = buffer_size;
    buffer_str.append(group_name, 0, strlen(group_name));
    buffer_size += strlen(group_name) + 1;

    cursor->mess_type_offset = buffer_size;
    buffer_str.append(mess_type, 0, strlen(mess_type));
    buffer_size += strlen(mess_type) + 1;

    cursor->send_host_name_offset = buffer_size;
    buffer_str.append(send_host_name, 0, strlen(send_host_name));
    buffer_size += strlen(send_host_name) + 1;

    cursor->recive_host_name_offset = buffer_size;
    buffer_str.append(recive_host_name, 0, strlen(recive_host_name));
    buffer_size += strlen(recive_host_name) + 1;

    cursor->message_offset = buffer_size;
    cursor->message_size = message_size;
    buffer_str.append(message, 0, message_size - 1);
    buffer_size += message_size;
}

void EventMessage::init_message_ptr()
{
    if (buffer_size <= sizeof(EventMessageCursor))
        return;
    cursor = (EventMessageCursor *)buffer_str.c_str();
    group_name = (const char *)(buffer_str.c_str() + cursor->group_name_offset);
    mess_type = (const char *)(buffer_str.c_str() + cursor->mess_type_offset);
    send_host_name = (const char *)(buffer_str.c_str() + cursor->send_host_name_offset);
    recive_host_name = (const char *)(buffer_str.c_str() + cursor->recive_host_name_offset);
    message = (const char *)(buffer_str.c_str() + cursor->message_offset);
    message_size = cursor->message_size;
}

/***********************************
 * class  EventMessageHandle
 * *******************************/

bool EventMessageHandle::init_handle(const char *host_config_path, const char *mess_config_path)
{
    if (this_free == true)
        this_free = false;
    else
        return false;

    read_config(host_config_path, mess_config_path);
    //init_group_infor_map();
}

bool EventMessageHandle::free_handle()
{
    if (this_free == false)
        this_free = true;
    else
        return false;
    //mess_config_doc.Clear();

    for (auto it = mess_group_map.begin(); it != mess_group_map.end(); it++)
    {
        delete it->second;
    }
}

bool EventMessageHandle::is_free()
{
    return this_free;
}

bool EventMessageHandle::is_init()
{
    return !this_free;
}


bool EventMessageHandle::register_recive_handler(const char * group_name, const char * mess_type,EventMessageHandle_RECIVE_CB cb,void * arg)
{
    EventMessage mess;
    mess.group_name = group_name;
    mess.mess_type = mess_type;
    if(!check_mess_type(&mess))
    {
        return false;
    }

    CallbackInfor * cb_infor_ptr = &mess_group_map[group_name]->mess_callback_map[mess_type];

    cb_infor_ptr->cb_function = cb;
    cb_infor_ptr->cb_arg = arg;

    return true;
}

int EventMessageHandle::readMessage(EventMessage *mess_ptr)
{
    if(check_mess_type(mess_ptr))
    {
        return -1;
    }

    auto & mess_type_map = mess_group_map[mess_ptr->group_name]->mess_callback_map;
    if(mess_type_map[mess_ptr->mess_type].unprocessed_mess_list.empty())
    {
        return 0;
    }

    mess_ptr->copy(mess_type_map[mess_ptr->mess_type].unprocessed_mess_list.front());
    return 1;
}


int EventMessageHandle::sendMessage(EventMessage *mess_ptr)
{
    if(check_mess_type(mess_ptr))
    {
        return -1;
    }

    LibeventHandle & event_handle = mess_group_map[mess_ptr->group_name]->handle;

    const char *  ip = get_ip(mess_ptr->recive_host_name);
    int port = get_port(mess_ptr->recive_host_name,mess_ptr->group_name);

    if(ip == NULL || port == 0)
    {
        return -1;
    }

    mess_ptr->send_host_name = local_host_name.c_str();
    mess_ptr->init_buffer_str();

    int connect_id = event_handle.get_connection_id(ip,port,true);

    event_handle.send(connect_id,mess_ptr->buffer_str.c_str(),mess_ptr->buffer_size);
}

void EventMessageHandle::read_config(const char *host_config_path, const char *mess_config_path)
{
    char readbuffer[FILE_IO_BUFFER_SIZE];

    //read host config
    rapidjson::Document host_document;
    FILE *host_file = fopen(host_config_path, "r");
    assert(host_file != NULL);
    memset(readbuffer, 0, FILE_IO_BUFFER_SIZE);
    rapidjson::FileReadStream read_stream1(host_file, readbuffer, FILE_IO_BUFFER_SIZE);
    host_document.ParseStream(read_stream1);
    fclose(host_file);
    // init host map
    for (auto &host : host_document.GetArray())
    {
        const char *host_name = host["name"].GetString();
        const char *host_ip = host["ip"].GetString();
        bool is_local = host["is_local"].GetBool();
        if (is_local)
        {
            local_host_name = host_name;
        }
        host_config_map[host_name] = host_ip;
    }

    // read message config
    rapidjson::Document mess_config_doc;
    FILE *config_file = fopen(mess_config_path, "r");
    assert(config_file != NULL);
    memset(readbuffer, 0, FILE_IO_BUFFER_SIZE);
    rapidjson::FileReadStream read_stream2(config_file, readbuffer, FILE_IO_BUFFER_SIZE);
    mess_config_doc.ParseStream(read_stream2);
    fclose(config_file);

    init_group_infor_map(mess_config_doc);
}

void EventMessageHandle::init_group_infor_map(rapidjson::Document &mess_config_doc)
{
    int len = mess_config_doc.Size();
    for (int i = 0; i < len; i++)
    {
        const char *group_name = mess_config_doc[i]["group_name"].GetString();
        int local_port = 0; //mess_config_doc[i]["hosts"][local_host_name.c_str()]["port"].GetInt();
        GroupInfor * group_ptr = new GroupInfor;
        mess_group_map[group_name] = group_ptr;

        for (auto &host_item : mess_config_doc[i]["hosts"].GetArray())
        {
            group_ptr->host_port_map[host_item["name"].GetString()] = host_item["port"].GetInt();
            // std::cout  << host_item["name"].GetString() << " : " << host_item["port"].GetInt()<< std::endl;
        }

        local_port = group_ptr->host_port_map[local_host_name];
        group_ptr->handle.init_handle(local_port);

        for (auto &mess_type : mess_config_doc[i]["mess_type"].GetArray())
        {
            CallbackInfor cb_infor;
            group_ptr->mess_callback_map[mess_type["name"].GetString()] = cb_infor;

            CallbackInfor *cb_infor_ptr = &group_ptr->mess_callback_map[mess_type["name"].GetString()];

            cb_infor_ptr->cb_function = NULL;

            cb_infor_ptr->cb_arg = NULL;

            cb_infor_ptr->unprocessed_mess_list.clear();

            // std::cout << (group_ptr->mess_callback_map.find(mess_type["name"].GetString()) != group_ptr->mess_callback_map.end() )<< std::endl;
        }

        // #ifdef  EVENT_MESS_HANDLE_DEBUG
        // std::cout << "----------------- Message Callback Map Debug ------------------- " << std::endl;
        // for(auto it = group_ptr->mess_callback_map.begin(); it!=group_ptr->mess_callback_map.end(); it++)
        // {
        // }
        // #endif //
    }
}

const char *EventMessageHandle::get_ip(const char *hostname)
{
    auto iterator = host_config_map.find(hostname);
    if (iterator == host_config_map.end())
    {
        return NULL;
    }
    else
    {
        return iterator->second.c_str();
    }
}

int EventMessageHandle::get_port(const char *hostname, const char *group_name)
{
    auto iterator = mess_group_map.find(group_name);
    if (iterator == mess_group_map.end())
    {
        return 0;
    }
    else
    {
        return mess_group_map[group_name]->host_port_map[hostname];
    }
}

LibeventHandle *EventMessageHandle::get_libeventhandle(const char *group_name)
{
    auto iterator = mess_group_map.find(group_name);
    if (iterator == mess_group_map.end())
    {
        return NULL;
    }
    else
    {
        return &iterator->second->handle;
    }
}

bool EventMessageHandle::check_mess_type(EventMessage * mess_ptr)
{
    if(mess_group_map.find(mess_ptr->group_name) == mess_group_map.end())
    {
        return false;
    }
    auto & mess_type_map = mess_group_map[mess_ptr->group_name]->mess_callback_map;
    if(mess_type_map.find(mess_ptr->mess_type) == mess_type_map.end())
    {
        return false;
    }
    return true;
}

bool EventMessageHandle::try_run_callback(EventMessage * mess_ptr)
{
    if(! check_mess_type(mess_ptr))
    {
        return false;
    }

    CallbackInfor * cb_infor_ptr = & mess_group_map[mess_ptr->group_name]->mess_callback_map[mess_ptr->mess_type];
    if(cb_infor_ptr->cb_function != NULL)
    {
        (*cb_infor_ptr->cb_function)(this,mess_ptr,cb_infor_ptr->cb_arg);
    }
    else
    {
        cb_infor_ptr->unprocessed_mess_list.push_back(*mess_ptr);
    }
    return true;
}

bool EventMessageHandle::read_callback_message_from_libevent(int connect_id,LibeventHandle *handle_ptr)
{
    // read one message from libevent
    EventMessage mess;
    int recive_size = handle_ptr->recive_str(connect_id,mess.buffer_str,false); // run in callback function,not wait
    if(recive_size <= 0)
    {
        return false;
    }
    mess.init_message_ptr();

    // try run some callback
    if(!try_run_callback(&mess))
    {
        return false;
    }

    return true;
}

void libevent_callback(NET_EVENT event_id, NetworkHandle *handle, int connect_id, void *arg)
{
    LibeventHandle *event_handle = (LibeventHandle *)handle;
    EventMessageHandle *mess_handle = (EventMessageHandle *)arg;
    //read message from libevent
    //get mess_type in message_type
    //call mess_handle->try_run_callback
    mess_handle->read_callback_message_from_libevent(connect_id,event_handle);
}