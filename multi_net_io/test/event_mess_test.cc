#include "event_mess_handle.h"

using namespace std;

int main(void)
{
    EventMessageHandle handler;
    // handler.read_config("./test/mess_config.json");
    // cout  << handler.mess_config_doc[0]["group_name"].GetString()<< endl;
    //  cout  << handler.mess_config_doc.Size()<< endl;

    handler.init_handle("./test/host_config.json","./test/mess_config.json");
    // string test_str = "468";
    // cout << sizeof(test_str) << endl;
    // cout << test_str.length() << endl;


    // cout << "Message Test" << endl;
    // cout << "---------- Message Send Test ------------" << endl;
    // EventMessage mess;
    // mess.prepare_send("g1","m12","host1","host2","abcdefg",7);
    // cout << "---------- Message Recive Test ------------" << endl;
    // EventMessage mess2;
    //  cout << mess.group_name << endl;
    //  cout << mess.mess_type << endl;
    //  cout << mess.send_host_name << endl;
    //  cout << mess.recive_host_name << endl;
    //  cout << mess.message << endl;
    //  cout << mess.message_size << endl;
}