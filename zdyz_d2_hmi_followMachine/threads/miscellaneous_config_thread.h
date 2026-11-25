#ifndef MISCELLANEOUS_CONFIG_THREAD_H
#define MISCELLANEOUS_CONFIG_THREAD_H

#include <QThread>
#include <string>
#include <vector>

using namespace std;

class MisCellConfig : public QThread
{
    Q_OBJECT
private:
    MisCellConfig();
public:
    ~MisCellConfig(){}

public:
    static MisCellConfig* getInstance(){
        if(pup == nullptr){
            pup = new MisCellConfig();
        }
        return pup;
    }

protected:
    void run();

public:
    void setGlobalVariable(string cmdline);
    void split(const string &s, vector<string> &tokens, const string &delimiters);

public:
    static MisCellConfig* pup;

};

#endif // MISCELLANEOUS_CONFIG_THREAD_H
