#include "miscellaneous_config_thread.h"
#include "../Resources/tcpclient.h"
#include <QDebug>

MisCellConfig* MisCellConfig::pup = nullptr;

extern TcpClient *tcp_client;

extern int update_flag_workface;

MisCellConfig::MisCellConfig()
{

}

void MisCellConfig::run()
{
    while(1)
    {
        char buf[128] = {0};
        if(read(tcp_client->sock_fd, buf, sizeof(buf)) > 0){
            qDebug() << "miscellconfig socket recieved:" << buf;
            printf("strlen(msg_buf.buf) = %d\n", strlen(buf));
            if(strlen(buf) <= 1)
            {
                continue;
            }
            string cmdline(buf);
            if(cmdline.find("update_flag_workface:") != string::npos)
            {
                setGlobalVariable(cmdline);
            }
            else{
                usleep(200000);
            }
        }
    }
}

void MisCellConfig::split(const string &s, vector<string> &tokens, const string &delimiters)
{
    string::size_type lastPos = s.find_first_not_of(delimiters, 0);
    string::size_type pos = s.find_first_of(delimiters, lastPos);
    while(lastPos != string::npos || pos != string::npos)
    {
        tokens.push_back(s.substr(lastPos, pos - lastPos));
        lastPos = s.find_first_not_of(delimiters, pos);
        pos = s.find_first_of(delimiters, lastPos);
    }
}

void MisCellConfig::setGlobalVariable(string cmdline)
{
    vector<string> vec;
    split(cmdline, vec, ":");
    update_flag_workface = atoi(vec[1].c_str());
    printf("#####update_flag_workface = %d\n", update_flag_workface);
}
