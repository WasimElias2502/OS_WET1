#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include "Commands.h"
#include <fcntl.h>


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

// TODO: Add your implementation for classes in Commands.h 

SmallShell::SmallShell() {
// TODO: add your implementation
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

  if ((firstWord.compare("pwd") == 0) || (firstWord.compare("pwd&") == 0)) {
    return new GetCurrDirCommand(cmd_line);
  }
  else if ((firstWord.compare("showpid") == 0) || (firstWord.compare("showpid&") == 0)) {
    return new ShowPidCommand(cmd_line);
  }
  else if ((firstWord.compare("cd") == 0) || (firstWord.compare("cd&") == 0)) {
      return new ChangeDirCommand(cmd_line,lastDir);
  }
  else if ((firstWord.compare("jobs") == 0) || (firstWord.compare("jobs&") == 0)) {
      return new JobsCommand(cmd_line,&jobsInShell);
  }
  else if ((firstWord.compare("quit") == 0) || (firstWord.compare("quit&") == 0)) {
      return new QuitCommand(cmd_line,&jobsInShell);
  }  else if ((firstWord.compare("kill") == 0) || (firstWord.compare("kill&") == 0)) {
      return new KillCommand(cmd_line,&jobsInShell);
  }
  else if ((firstWord.compare("fg") == 0) || (firstWord.compare("fg&") == 0)) {
      return new ForegroundCommand(cmd_line,&jobsInShell);
  }
  else if ((firstWord.compare("chprompt") == 0) || (firstWord.compare("chprompt&") == 0)) {
      return new ChPromptCommand(cmd_line);
  }
  else {
    return new ExternalCommand(cmd_line);
  }

  return nullptr;
}

void SmallShell::executeCommand(const char *cmd_line) {
  // TODO: Add your implementation here
  // for example:
   Command* cmd = CreateCommand(cmd_line);
   cmd->execute();
  // Please note that you must fork smash process for some commands (e.g., external commands....)
}



/////////////////////////////////Command//////////////////////////////////////////////

Command::Command(const char *cmd_line) : numArgs(0) {

    numArgs = _parseCommandLine(cmd_line,arguments);

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
                cerr<< "smash error: getcwd error" << endl ;
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
             cerr<< "smash error: getcwd error" << endl ;
         }
         return;
    }

    else{
         char* buffer = new char[MAX_CHARACTERS_INDIR+1];
         if(getcwd(buffer,sizeof(buffer)) != nullptr){
             lastDir = &buffer;
         }
         else{
             cerr<< "smash error: getcwd error" << endl ;
         }
        chdir(arguments[1]);
    }
    return;
}

///////////////////////PWD////////////////////////////////////////////////////////////

GetCurrDirCommand::GetCurrDirCommand(const char *cmd_line) : BuiltInCommand(cmd_line){}

void GetCurrDirCommand::execute() {

    char* buffer = new char[MAX_CHARACTERS_INDIR+1];
    if(getcwd(buffer,sizeof(buffer)) != nullptr){
        cout<< buffer << endl;
        delete buffer;
    }
    else{
        cerr<<"smash error: getcwd error" << endl ;
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
        SmallShell::prompt = "smash";
    }
    else{
        SmallShell::prompt = arguments[1];
    }
}


//////////////////////JobsListClass///////////////////////////////////

KillCommand::KillCommand(const char *cmd_line, JobsList *jobs) : BuiltInCommand(cmd_line) , jobs(jobs){}

void KillCommand::execute() {

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


void JobsList::addJob(Command *cmd, bool isStopped) {

    removeFinishedJobs();
    numOfJobs = listOfJobs.size();

    listOfJobs.push_back(JobEntry(listOfJobs.))

}



