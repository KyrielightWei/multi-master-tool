#include "event_mess_handle.h"
#include "test_config.h"

using namespace std;

int main(void)
{
    EventMessageHandle handler;

    string host_config_path = std::string(TEST_SOURCE_DIR) + "host_config1.json";
    string mess_config_path = std::string(TEST_SOURCE_DIR) + "mess_config.json";
    cout << "host config file path = " << host_config_path << endl << "message config file path = " << mess_config_path << endl;
    handler.init_handle(host_config_path.c_str(),mess_config_path.c_str());


    cout << "Message Client Test" << endl;

 ///   cout << "---------- Message Callback Recive Test ------------" << endl;

    int count = 0;
    int return_code =0;
    while(1){
        cout << "---------- Message Constantly Send Test ------------" << endl;
        EventMessage while_mess;
        string while_str = "send count : " + std::to_string(count);
        while_mess.prepare_send("admin","heart","host2",while_str.c_str(),while_str.size());
        return_code = handler.sendMessage(&while_mess);
        cout << "sendMessage return_code : " << return_code << endl;
        if(return_code < 0)
        {
             cout << "send error" << std::endl;
             cout << MessageError::getEventErrorStr(while_mess.error_no) << endl;
        }
        else
        {
            cout << while_mess.message << std::endl;
            count++;
        }

        sleep(5);
    }
}