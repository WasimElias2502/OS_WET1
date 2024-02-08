#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <vector>
#include <memory>
#define COMMAND_ARGS_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)
#define MAX_CHARACTERS_INDIR (80)
const std::string WHITESPACE = " \n\r\t\f\v";

class Command {
protected:
// TODO: Add your data members
    char* arguments[COMMAND_MAX_ARGS+1];
    int numArgs;
public:
    char cmd_line[COMMAND_ARGS_MAX_LENGTH+1];
    Command(const char* cmd_line);
    virtual ~Command();
    virtual void execute() = 0;
    //virtual void prepare();
    //virtual void cleanup();
    // TODO: Add your extra methods if needed
};

class BuiltInCommand : public Command {

public:
    BuiltInCommand(const char* cmd_line);
    virtual ~BuiltInCommand() {}
};

class ExternalCommand : public Command {

public:
    //const char* OriginalCmdLine;
    ExternalCommand(const char* cmd_line);
    virtual ~ExternalCommand() {}
    void execute() override;
};

class PipeCommand : public Command {
    // TODO: Add your data members
public:
    PipeCommand(const char* cmd_line);
    virtual ~PipeCommand() {}
    void execute() override;
};

class RedirectionCommand : public Command {
    // TODO: Add your data members

    int fdOfStdOut;


public:
    explicit RedirectionCommand(const char* cmd_line);
    virtual ~RedirectionCommand() {}
    void execute() override;
    void prepare() ;
    void cleanup() ;
};

class ChangeDirCommand : public BuiltInCommand {
// TODO: Add your data members
    char* lastDir;
public:
    ChangeDirCommand(const char* cmd_line, char* plastPwd);
    virtual ~ChangeDirCommand() {}
    void execute() override;
};

class GetCurrDirCommand : public BuiltInCommand {
public:
    GetCurrDirCommand(const char* cmd_line);
    virtual ~GetCurrDirCommand() {}
    void execute() override;
};

class ShowPidCommand : public BuiltInCommand {
public:
    ShowPidCommand(const char* cmd_line);
    virtual ~ShowPidCommand() {}
    void execute() override;
};

class JobsList;
class QuitCommand : public BuiltInCommand {
// TODO: Add your data members

    JobsList* jobs;

public:
    QuitCommand(const char* cmd_line, JobsList* jobs);
    virtual ~QuitCommand() {}
    void execute() override;
};




class JobsList {
public:
    class JobEntry {

    public:
        // TODO: Add your data members
        int jobId ;
        pid_t processId;
        Command* cmd_line;



        JobEntry(int jobId , pid_t processId , Command* cmd_line) : jobId(jobId) , processId(processId) , cmd_line(cmd_line) {}
        ~JobEntry (){
            if(cmd_line != nullptr) delete cmd_line;
        }

        bool operator>( const JobEntry& job2){
            return jobId > job2.jobId;
        };
        bool operator<( const JobEntry& job2){
            return jobId < job2.jobId;
        }
        bool operator==(const JobEntry& job2){
            return jobId == job2.jobId;
        }



    };
    // TODO: Add your data members




    std::vector<std::shared_ptr<JobEntry>> listOfJobs;
    int numOfJobs ;

public:
    JobsList();
    ~JobsList();
    void addJob(Command* cmd,pid_t newJobpid,bool isStopped = false); // implemented
    void printJobsList(); // implemented
    void killAllJobs(); // implemented
    void removeFinishedJobs();// implemented
    JobEntry * getJobById(int jobId); // implemented
    void removeJobById(int jobId); // implemented
    JobEntry * getLastJob(int* lastJobId);
    JobEntry *getLastStoppedJob(int *jobId);
    // TODO: Add extra methods or modify exisitng ones as needed
};

class JobsCommand : public BuiltInCommand {
    // TODO: Add your data members


public:
    JobsList* jobs;
    JobsCommand(const char* cmd_line, JobsList* jobs);
    virtual ~JobsCommand() {}
    void execute() override;
};


class KillCommand : public BuiltInCommand {
    // TODO: Add your data members

    JobsList* jobs ;

public:
    KillCommand(const char* cmd_line, JobsList* jobs);
    virtual ~KillCommand() {}
    void execute() override;
};

class ForegroundCommand : public BuiltInCommand {
    // TODO: Add your data members

    JobsList* jobs;

public:
    ForegroundCommand(const char* cmd_line, JobsList* jobs);
    virtual ~ForegroundCommand() {}
    void execute() override;
};

class ChmodCommand : public BuiltInCommand {
public:
    ChmodCommand(const char* cmd_line);
    virtual ~ChmodCommand() {}
    void execute() override;
};



class ChPromptCommand : public BuiltInCommand {

public:
    ChPromptCommand(const char* cmd_line);
    virtual ~ChPromptCommand(){}
    void execute() override;
};


class SmallShell {
private:
    // TODO: Add your data members

    JobsList jobsInShell;

    SmallShell();
public:
    char* lastDir;
    std::string prompt ;
    static pid_t smashPid ;
    Command *CreateCommand(const char* cmd_line);
    SmallShell(SmallShell const&)      = delete; // disable copy ctor
    void operator=(SmallShell const&)  = delete; // disable = operator
    static SmallShell& getInstance() // make SmallShell singleton
    {
        static SmallShell instance; // Guaranteed to be destroyed.
        // Instantiated on first use.
        return instance;
    }
    ~SmallShell();
    void executeCommand(const char* cmd_line);
    // TODO: add extra methods as needed
};

#endif //SMASH_COMMAND_H_
