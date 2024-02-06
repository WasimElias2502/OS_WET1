#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include "Commands.h"
#include <fcntl.h>
#include <algorithm>


using namespace std;

#if 0
#define FUNC_ENTRY()  \
  cout << __PRETTY_FUNCTION__ << " --> " << endl;

#define FUNC_EXIT()  \
  cout << __PRETTY_FUNCTION__ << " <-- " << endl;
#else
#define FUNC_ENTRY()
#define FUNC_EXIT()
#endif


string _ltrim(const std::string& s)
{
    size_t start = s.find_first_not_of(WHITESPACE);
    return (start == std::string::npos) ? "" : s.substr(start);
}

string _rtrim(const std::string& s)
{
    size_t end = s.find_last_not_of(WHITESPACE);
    return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

string _trim(const std::string& s)
{
    return _rtrim(_ltrim(s));
}

int _parseCommandLine(const char* cmd_line, char** args) {
    FUNC_ENTRY()
    int i = 0;
    std::istringstream iss(_trim(string(cmd_line)).c_str());
    for(std::string s; iss >> s; ) {
        args[i] = (char*)malloc(s.length()+1);
        memset(args[i], 0, s.length()+1);
        strcpy(args[i], s.c_str());
        args[++i] = NULL;
    }
    return i;

    FUNC_EXIT()
}

bool _isBackgroundComamnd(const char* cmd_line) {
    const string str(cmd_line);
    return str[str.find_last_not_of(WHITESPACE)] == '&';
}

void _removeBackgroundSign(char* cmd_line) {
    const string str(cmd_line);
    // find last character other than spaces
    unsigned int idx = str.find_last_not_of(WHITESPACE);
    // if all characters are spaces then return
    if (idx == string::npos) {
        return;
    }
    // if the command line does not end with & then return
    if (cmd_line[idx] != '&') {
        return;
    }
    // replace the & (background sign) with space and then remove all tailing spaces.
    cmd_line[idx] = ' ';
    // truncate the command line string up to the last non-space character
    cmd_line[str.find_last_not_of(WHITESPACE, idx) + 1] = 0;
}


char* GetParentDirectory(const char* directoryPath) {
    std::string path(directoryPath);

    size_t lastSeparator = path.find_last_of("/\\");

    if (lastSeparator != std::string::npos) {
        path = path.substr(0, lastSeparator);
    }

    char* result = new char[path.length() + 1];
    strcpy(result, path.c_str());

    return result;
}



bool IsComplexExternalCmd(const char* cmd_line){
    int i=0;
    while(cmd_line[i] != '\0'){
        if(cmd_line[i] == '*' || cmd_line[i] == '?') return true;
    }
    return false;
}


/////// override =1 append = 2 not redirection = -1
int checkRedirection(const char* cmd_line){
    char* arguments[COMMAND_MAX_ARGS+1];
    for(int i=0 ; i<=COMMAND_MAX_ARGS ; i++) arguments[i] = nullptr;
    int numArgs = _parseCommandLine(cmd_line, arguments);
    if (numArgs < 3) return -1;
    if (strcmp(arguments[numArgs-2],">") == 0){
        for(int i=0 ; i<=COMMAND_MAX_ARGS ; i++){
            if(arguments[i] != nullptr) delete arguments[i] ;
        }
        return 1;
    }

    else if(strcmp(arguments[numArgs-2],">>") == 0){
        for(int i=0 ; i<=COMMAND_MAX_ARGS ; i++){
            if(arguments[i] != nullptr) delete arguments[i] ;
        }
        return 2;
    }

    for(int i=0 ; i<=COMMAND_MAX_ARGS ; i++){
        if(arguments[i] != nullptr) delete arguments[i] ;
    }
    return -1;



}

// TODO: Add your implementation for classes in Commands.h


SmallShell::SmallShell() {
// TODO: add your implementation
    prompt = "smash";
}

SmallShell::~SmallShell() {
// TODO: add your implementation
}

/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/
Command * SmallShell::CreateCommand(const char* cmd_line) {
    // For example:

    string cmd_s = _trim(string(cmd_line));
    string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));

    char* BuiltInIgnoreAmpersand = new char[COMMAND_ARGS_MAX_LENGTH];
    strcpy(BuiltInIgnoreAmpersand,cmd_line);
    _removeBackgroundSign(BuiltInIgnoreAmpersand);
    string Copy = BuiltInIgnoreAmpersand;
    Copy = _trim(Copy);
    strcpy(BuiltInIgnoreAmpersand , Copy.c_str());


    ////Check if command redirection
    if(checkRedirection(cmd_line) == 1 || checkRedirection(cmd_line) == 2){
        char* newCmd = new char[COMMAND_ARGS_MAX_LENGTH+1];
        strcpy(newCmd,cmd_line);
        return new RedirectionCommand(newCmd);
    }

    if ((firstWord.compare("pwd") == 0) || (firstWord.compare("pwd&") == 0)) {
        return new GetCurrDirCommand(BuiltInIgnoreAmpersand);
    }
    else if ((firstWord.compare("showpid") == 0) || (firstWord.compare("showpid&") == 0)) {
        return new ShowPidCommand(BuiltInIgnoreAmpersand);
    }
    else if ((firstWord.compare("cd") == 0) || (firstWord.compare("cd&") == 0)) {
        return new ChangeDirCommand(BuiltInIgnoreAmpersand,lastDir);
    }
    else if ((firstWord.compare("jobs") == 0) || (firstWord.compare("jobs&") == 0)) {
        return new JobsCommand(BuiltInIgnoreAmpersand,&jobsInShell);
    }
    else if ((firstWord.compare("quit") == 0) || (firstWord.compare("quit&") == 0)) {
        return new QuitCommand(BuiltInIgnoreAmpersand,&jobsInShell);
    }  else if ((firstWord.compare("kill") == 0) || (firstWord.compare("kill&") == 0)) {
        return new KillCommand(BuiltInIgnoreAmpersand,&jobsInShell);
    }
    else if ((firstWord.compare("fg") == 0) || (firstWord.compare("fg&") == 0)) {
        return new ForegroundCommand(BuiltInIgnoreAmpersand,&jobsInShell);
    }
    else if ((firstWord.compare("chprompt") == 0) || (firstWord.compare("chprompt&") == 0)) {
        return new ChPromptCommand(BuiltInIgnoreAmpersand);
    }
    else {
        ////////////

        pid_t currPid = fork();

        if (currPid == 0){
            return new ExternalCommand(cmd_line);
        }
        else if(currPid > 0) {
            if (_isBackgroundComamnd(cmd_line)){
                char cmdWOBackGround[COMMAND_ARGS_MAX_LENGTH];
                strcpy(cmdWOBackGround,cmd_line);
                _removeBackgroundSign(cmdWOBackGround);
                strcpy(cmdWOBackGround, _trim(cmdWOBackGround).c_str());
                jobsInShell.addJob(new ExternalCommand(cmdWOBackGround),currPid);
            }
            else{
                int status;
                pid_t childProcess = waitpid(currPid, &status,0);
                if(childProcess == -1){
                    perror( "smash error: waitpid failed");
                }
                else{
                    return nullptr;
                }
            }
        }
        else{
            perror("smash error: fork failed");
        }

    }

    return nullptr;
}

void SmallShell::executeCommand(const char *cmd_line) {
    // TODO: Add your implementation here
    // for example:
    Command* cmd = CreateCommand(cmd_line);
    if(cmd != nullptr) {
        jobsInShell.removeFinishedJobs();
        cmd->execute();
        // Please note that you must fork smash process for some commands (e.g., external commands....)

        delete cmd;
    }
}



/////////////////////////////////Command//////////////////////////////////////////////

Command::Command(const char *cmd_line) : numArgs(0) , cmd_line(cmd_line){

    for(int i=0 ; i<=COMMAND_MAX_ARGS ; i++){
        arguments[i] = nullptr;
    }

    numArgs = _parseCommandLine(cmd_line,arguments);

}

Command::~Command() {
    for(int i=0 ; i<=COMMAND_MAX_ARGS ; i++){
        if(arguments[i] != nullptr) delete arguments[i] ;
    }

    delete cmd_line;
}



////////////////////////////BuiiltInCommand//////////////////////////////////////////

BuiltInCommand::BuiltInCommand(const char *cmd_line) : Command(cmd_line){}





/////////////////////////Change Directory/////////////////////////////////////////////

ChangeDirCommand::ChangeDirCommand(const char *cmd_line, char **plastPwd) : BuiltInCommand(cmd_line) {
    lastDir = plastPwd ;
}

void ChangeDirCommand::execute() {

    ////Errors
    if(Command::numArgs != 2){
        if(numArgs>2){
            cerr<<"smash error: cd: too many arguments"<< endl;
        }
        return;
    }
    /////go to last directory
    if(arguments[1] == "-"){
        if(lastDir == nullptr){////
            cerr<<"smash error: cd: OLDPWD not set"<< endl;
            return;
        }
        else{
            char* buffer = new char[MAX_CHARACTERS_INDIR+1];
            if(getcwd(buffer,sizeof(buffer)) != nullptr){
                chdir(*lastDir);                                                                                           ///////////////check if failed later
                if (*lastDir) {
                    delete lastDir;
                    lastDir = &buffer;
                    return;
                }
            }
            else{
                perror("smash error: getcwd failed");
                return;
            }
        }
        return;
    }

        ///// retrun to parent directory

    else if( arguments[1] == ".."){
        char* buffer = new char[MAX_CHARACTERS_INDIR+1];
        if(getcwd(buffer,sizeof(buffer)) != nullptr){
            char* parentPath = GetParentDirectory(buffer);
            chdir(parentPath);                                                                                         ////////////SYSFAIL check
            if(lastDir != nullptr) delete lastDir;
            lastDir = &buffer;
            if(parentPath != nullptr) delete parentPath;
        }
        else{
            perror("smash error: getcwd failed");
        }
        return;
    }

    else{
        char* buffer = new char[MAX_CHARACTERS_INDIR+1];
        if(getcwd(buffer,sizeof(buffer)) != nullptr){
            lastDir = &buffer;
        }
        else{
            perror("smash error: getcwd failed");
        }
        chdir(arguments[1]);
    }
    return;
}

///////////////////////PWD////////////////////////////////////////////////////////////

GetCurrDirCommand::GetCurrDirCommand(const char *cmd_line) : BuiltInCommand(cmd_line){}

void GetCurrDirCommand::execute() {

    char buffer[MAX_CHARACTERS_INDIR+1];
    if(getcwd(buffer,sizeof(buffer)) != nullptr){
        cout<< buffer << endl;

    }
    else{
        perror("smash error: getcwd failed");
    }
    return;
}


///////////////////////ShowPIDCommand////////////////////////////////////////////



pid_t SmallShell::smashPid = getpid();

ShowPidCommand::ShowPidCommand(const char *cmd_line) : BuiltInCommand(cmd_line){}

void ShowPidCommand::execute() {
    cout<< "smash pid is " <<SmallShell::smashPid << endl;
}



//////////////////////Chprompt////////////////////////////////////////////////

ChPromptCommand::ChPromptCommand(const char *cmd_line) : BuiltInCommand(cmd_line){}

void ChPromptCommand::execute() {
    if (numArgs == 1){
        SmallShell::getInstance().prompt = "smash";
    }
    else{
        SmallShell::getInstance().prompt = arguments[1];
    }
}


//////////////////////JobsListClass///////////////////////////////////

KillCommand::KillCommand(const char *cmd_line, JobsList *jobs) : BuiltInCommand(cmd_line) , jobs(jobs){}

void KillCommand::execute() {

    ////Arguments are invalid

    if(numArgs != 3){
        cerr << "smash error: kill:  invalid arguments"<< endl;
        return;
    }

    if(arguments[1][0] != '-') {
        cerr << "smash error: kill:  invalid arguments"<< endl;
        return;
    }
    int job_id = 0;
    int signum = 0;

    try{
        job_id = stoi(arguments[2]);
        signum = stoi(&(arguments[1][1]));
    }
    catch(...){
        cerr << "smash error: kill:  invalid arguments"<< endl;
        return;
    }


    JobsList::JobEntry* my_job = jobs->getJobById(job_id);

    /// Job Not Found
    if(my_job == nullptr) {
        cerr << "smash error: kill: job-id " << job_id << " does not exist" << endl;
        return;
    }

    //////Correct

    if(kill(my_job->processId, signum) != 0) {
        perror("smash error: kill failed");
    }
    else{
        cout<<"signal number "<<signum<<" was sent to pid "<<my_job->processId<<endl;
    }
}


//////////////////////Quit///////////////////////////////////////

QuitCommand::QuitCommand(const char *cmd_line, JobsList *jobs) : BuiltInCommand(cmd_line) , jobs(jobs){}

void QuitCommand::execute() {

    char* killArg = arguments[1];

    ///Kill not specified Or Argument Specified but not kill
    if(killArg == NULL  || killArg != "kill"){
        exit(0);
    }


        /// Argument specified and its kill Argument
    else{

        cout<<"smash: sending SIGKILL signal to "<<jobs->listOfJobs.size()<<" jobs:"<<endl;

        auto iterator = jobs->listOfJobs.begin();

        for(; iterator != jobs->listOfJobs.end() ; ++iterator){
            cout << iterator->processId << ": " << iterator->cmd_line << endl;
        }
    }
}



////////////////ForeGround////////////////////////////////

ForegroundCommand::ForegroundCommand(const char *cmd_line, JobsList *jobs) : BuiltInCommand(cmd_line) , jobs(jobs){}

void ForegroundCommand::execute() {
    char* jobIdArg = arguments[1];
    int jobIdToMove = 0;
    int status = 0;

    ///Inavild Arguments
    if(numArgs != 1 && numArgs!= 2){
        cerr<<"smash error: fg: invalid arguments"<<endl;
        return;
    }


    ///Optional Argument not specified

    if(jobIdArg == NULL){

        auto maxElement = std::max_element(jobs->listOfJobs.begin(), jobs->listOfJobs.end());

        // Check if the vector is not empty before accessing the result
        if (maxElement != jobs->listOfJobs.end()) {

            cout<<cmd_line<<" "<<maxElement->processId<<endl;

            ///Wait Failed
            if(waitpid(maxElement->processId,&status,0) == -1){
                perror("smash error: waitpid failed");
                return;
            }

            jobs->removeJobById(maxElement->jobId);
            return;

        } else {
            cerr<<"smash error: fg:jobs list is empty"<<endl;
            return;
        }
    }


        /////Optional Argument is specified
    else {

        try {
            jobIdToMove = stoi(jobIdArg);
        }
        catch (...) {
            cerr << "smash error: fg: invalid arguments" << endl;
            return;
        }

        /////Argument is specified and a valid number

        auto iterator = jobs->listOfJobs.begin();
        for(; iterator != jobs->listOfJobs.end(); ++iterator){
            if(iterator->jobId == jobIdToMove);
            break;
        }

        ///didnt find job with jobIdToMove
        if(iterator == jobs->listOfJobs.end()){
            cerr << "smash error: fg: job-id " << jobIdToMove << " does not exist" << endl;
            return;
        }

            ///found job with jobIdToMove
        else{

            cout<<cmd_line<<" "<<iterator->processId << endl;

            ///Wait Failed
            if(waitpid(iterator->processId,&status,0) == -1){
                perror("smash error: waitpid failed");
                return;
            }

            jobs->removeJobById(iterator->jobId);
            return;
        }
    }
}




///////////////////////////jobs//////////////////////////////////////////

JobsCommand::JobsCommand(const char *cmd_line, JobsList *jobs) : BuiltInCommand(cmd_line) ,  jobs(jobs) {}

void JobsCommand::execute() {
    if (jobs == nullptr){
        return;
    }
    jobs->removeFinishedJobs();
    jobs->printJobsList();
}





//////////////////////JobsList///////////////////////////////////////////////

JobsList::JobsList() : numOfJobs(0), listOfJobs(){}
JobsList::~JobsList() noexcept {}



///// for the jobs command not for quitkill
void JobsList::printJobsList(){


    auto iterator = listOfJobs.begin();
    for ( ; iterator != listOfJobs.end() ; ++iterator){
        cout << "[" << iterator->jobId << "] " << iterator->cmd_line->cmd_line << endl;
    }
}

void JobsList::addJob(Command *cmd,pid_t newJobpid ,bool isStopped) {

    removeFinishedJobs();
    numOfJobs = listOfJobs.size();

    ///Find Maximum job in list
    int MaxJobId = 0;
    auto maxElement = std::max_element(listOfJobs.begin(), listOfJobs.end());
    if (maxElement != listOfJobs.end()) {
        MaxJobId = maxElement->jobId;
    }
    else {
        MaxJobId=0;
    }

    ///////adding the new job
    listOfJobs.push_back(JobEntry(MaxJobId+1,newJobpid, cmd));

}

JobsList::JobEntry * JobsList::getJobById(int jobId){
    auto iterator = listOfJobs.begin();
    for ( ; iterator != listOfJobs.end() ; ++iterator){
        if (iterator->jobId == jobId){
            return &(*iterator);
        }
    }
    return nullptr;
}

void JobsList::removeJobById(int jobId){
    auto iterator = listOfJobs.begin();
    for ( ; iterator != listOfJobs.end() ; ++iterator){
        if (iterator->jobId == jobId){
            listOfJobs.erase(iterator);
            return;
        }
    }
}


void JobsList::killAllJobs(){
    auto iterator = listOfJobs.begin();
    for( ;iterator != listOfJobs.end() ; ++iterator){
        if(kill(iterator->processId , SIGKILL) == -1){
            perror("smash error: kill failed");
        }
    }
}

void JobsList::removeFinishedJobs(){
    pid_t smash_Pid = SmallShell::getInstance().smashPid;
    if(getpid() != smash_Pid){
        return;
    }

    auto iterator = listOfJobs.begin();
    pid_t deadProcess;

    for( ; iterator != listOfJobs.end() ; ++iterator){

        deadProcess =  waitpid(iterator->processId, NULL, WNOHANG);

        ///// is Zombie
        if(deadProcess > 0){
            listOfJobs.erase(iterator);
        }
            //// waitpid
        else if (deadProcess == -1){
            perror("smash error: waitpid failed");
            return;
        }
        else if (deadProcess == 0){
            return;
        }
    }
}



//////////////////////////////////External Command//////////////////////////////////////////////

ExternalCommand::ExternalCommand(const char *cmd_line) : Command(cmd_line) , OriginalCmdLine(cmd_line){}

void ExternalCommand::execute() {

    //////////Command is Complex
    if(IsComplexExternalCmd(cmd_line)){
        char* cmd_line_not_const = new char[COMMAND_ARGS_MAX_LENGTH+1];
        strcpy(cmd_line_not_const, cmd_line);
        char CFlag[3] = {'-','c','\0'};
        char* argv[] = { CFlag, cmd_line_not_const , nullptr};           ///////////////////////////////////
        if(execv("/bin/bash" , argv)==-1) perror("smash error: execv failed");
    }

        //////////////Command not complex
    else{
        int res = execv(arguments[0] ,arguments);
        if(res == -1) perror("smash error: execv failed");
    }
}


///Redirection Command////////////////////////

char* cutCMD(const char* cmd_line){
    int i=0;
    std::string cmd_new(cmd_line);
    cmd_new = cmd_new.substr(0, cmd_new.find_first_of('>')-1);
    cmd_new = _trim(cmd_new);
    char* ret_cmd;
    strcpy(ret_cmd, cmd_new.c_str());
    return ret_cmd;
}

RedirectionCommand::RedirectionCommand(const char *cmd_line) : Command(cmd_line){}

void RedirectionCommand::execute() {

    ///prepare the output stream
    prepare();

    ///Command execution
    const char* cuttedCMD = cutCMD(cmd_line);
    Command* command = SmallShell::getInstance().CreateCommand(cuttedCMD);

    if(command != nullptr) {
        command->execute();
        delete command;
    }

    ///cleanup the output stream
    cleanup();
}

///Preparing the output stream
void RedirectionCommand::prepare() {

    try {
        fdOfStdOut = dup(1);
    }catch(...){
        perror("smash error: dup failed");
        return;
    }

    ////CLOSE FAILED
    if(close(1) == -1) {
        perror("smash error: close failed");
        return;
    }

    int output;

    ////override = 1
    if (checkRedirection(cmd_line) == 1){
        output = open(arguments[numArgs-1],O_WRONLY | O_CREAT);
        if(output == -1){
            perror("smash error: open failed");
            return;
        }
    }

        /////append = 2
    else if(checkRedirection(cmd_line) == 2) {
        output = open(arguments[numArgs - 1], O_WRONLY | O_APPEND | O_CREAT);
        if (output == -1) {
            perror("smash error: open failed");
            return;
        }
    }
}

void RedirectionCommand::cleanup() {

    ////CLOSE FAILED
    if(close(1) == -1) {
        perror("smash error: close failed");
        return;
    }

    /////return stdout to the original fd
    try {
        dup(fdOfStdOut);
    }catch(...){
        perror("smash error: dup failed");
        return;
    }

    /////Closing the current stdOut fd
    if(close(fdOfStdOut) == -1) {
        perror("smash error: close failed");
        return;
    }
}



////////////////////////Chmod Command//////////////////////////////////////











