/**
 *  Test rapidjson basic function
 * 
 * **/

#include<iostream>
#include"json_packet.h"

using namespace std;

struct TestStruct
{
    char  space1[5];
    const char * ptr1;
   
    const char * ptr2;
    char  space2[5];
     char i;
     char ch;
};


void initTestBuffer(char * ch,int size,int val)
{
    for(int i =0; i<size; i++)
    {
        ch[i] = '0' + val;
    }
}


int main(void) 
{
    cout << "********** DynamicBuffer Test **********" << endl;
    DynamicBuffer dy;

    char tb1[128];
    char tb2[256];
    char tb3[512];
    char tb4[1024];
    char tb5[1524];

    initTestBuffer(tb1,128,1);
    initTestBuffer(tb2,256,2);
    initTestBuffer(tb3,512,3);
    initTestBuffer(tb4,1024,4);
    initTestBuffer(tb5,1524,5);

    dy.append(tb1,128);
     dy.append(tb2,256);
      dy.append(tb3,512);
    dy.print_buffer();

    char ch1,ch2,ch3;
    ch1 = *dy.get_address(383);
    ch2 = *dy.get_address(384);
    ch3 = *dy.get_address(385);
    

    cout << "ch1 = " << ch1 << std::endl;
    cout << "ch2 = " << ch2 << std::endl;
    cout << "ch3 = " << ch3 << std::endl;


    cout << "********** DynamicBuffer Test END **********" << endl;

    cout  << endl << "********** JSON Test **********" << endl;

    JsonPacket json;
    json.set_packet_header("from ip",7);

    json.set_packet_item("first","hello",5,JsonPacket::PacketItemType::FIRST);
    json.set_packet_item("second","world",5);
    json.set_packet_item("third",tb1,10,JsonPacket::PacketItemType::LAST);

    json.set_note_for_ptr("third",0,"575",3,JsonPacket::PacketItemType::FIRST);
    json.set_note_for_ptr("second",23,"989",3,JsonPacket::PacketItemType::LAST);
    cout << "JSON STRING : " << json.get_string_ptr() << endl;

    json.parse(json.get_string_ptr());

    json.printdy();

    cout  << endl << "********** JSON Test END **********" << endl;

    cout  << endl << "********** Struct Parse Test **********" << endl;

   // JsonPacket & json2 = json;

  // json.clear_packet();
    TestStruct testSTRUCT;
    int testInt1 = 123456796;
    int testInt2 = 45646887;
    std::string testStr = "hello world   ";
    std::string testStr1 = " xml ";
    std::string testStr2 = " java ";

    testSTRUCT.i = 'Z';
    testSTRUCT.ch = 'P';
    memset(testSTRUCT.space1,' ',5);
    memset(testSTRUCT.space2,' ',5);
    testSTRUCT.ptr1 = testStr2.c_str() + 3;
    testSTRUCT.ptr2 = testStr.c_str() + 8;

    cout << "testSTRUCT----------" << sizeof(char *) << endl;
    for(int q = 0;q<sizeof(testSTRUCT);q++)
    {
        cout << " " <<*(((char *)&testSTRUCT)+q) <<" " ;
    }
    cout << endl;

    json.clear_packet();

    cout << "JSON STRUCT STRING : " << json.get_string_ptr() << endl;

     json.printdy();


    json.set_packet_header("MY STRUCT TEST",14);
    json.set_packet_item("testInt1",testStr2.c_str(),6,JsonPacket::PacketItemType::FIRST);
    json.set_packet_item("testStr",testStr.c_str(),14);
    json.set_packet_item("testSTRUCT",(const char *)&testSTRUCT,sizeof(TestStruct),JsonPacket::PacketItemType::LAST);

     json.set_note_for_ptr("testSTRUCT",9,(const char *)testSTRUCT.ptr2,sizeof(char),JsonPacket::PacketItemType::FIRST);
    json.set_note_for_ptr("testSTRUCT",4, (const char *)testSTRUCT.ptr1 ,sizeof(char),JsonPacket::PacketItemType::LAST);

   

     cout << "JSON STRUCT STRING : " << json.get_string_ptr() << endl;


    json.parse(json.get_string_ptr());

    json.printdy();

    cout  << endl << "********** Struct Parse Test **********" << endl;

    return 0;
}