#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <filesystem>
#include <cstdlib>
#include <fstream>

std::vector<std::string> split_string(std::string paths, char delimiter) {
    std::stringstream ss(paths);
    std::string token;
    std::vector<std::string> return_vect;
    while(getline(ss, token, delimiter)){
      return_vect.push_back(token);
    }
    return return_vect;
}

std::string check_path(std::string type_text, std::vector<std::string> path) {
  std::string filepath;
  for(int i = 0; i < path.size(); i++){
    filepath = path[i] + '/' + type_text;
    if (std::filesystem::exists(filepath)) {
      return filepath; 
    }
  }
  return "";
}

int main() {
    // Flush after every std::cout / std:cerr
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

    std::cout << "$ ";
    std::string input;
    std::vector<std::string> commands{"echo", "exit", "type", "pwd", "cd"};
    std::string path_env = std::getenv("PATH");
    std::vector<std::string> path = split_string(path_env, ':');

    while (std::getline(std::cin, input) && input.find("exit") != 0) {
        if (input.find("echo") == 0) {
          // Handle echo cases
          std::string echo_text = input.substr(5);
          std::cout << echo_text << std::endl;
        } else if (input.find("type") == 0) {
          std::string type_text = input.substr(5);
          // See if type_text is in the list of commands
          if (std::find(commands.begin(), commands.end(), type_text) != commands.end()) {
            std::cout << type_text << " is a shell builtin\n";
          } else {
            // Check path for non-command type input
            std::string exe = check_path(type_text, path);
            if (!exe.empty()){
              std::cout << type_text << " is " << exe << "\n";
            } else {
              std::cout << type_text << ": not found\n";
            }
          }
        } else if (input.find("pwd") == 0) {
          //print working directory
          std::string cwd = std::filesystem::current_path();
          std::cout << cwd << "\n";
        } else if (input.find("cd") == 0) {
          // handle cd commands
          std::string cd = input.substr(3);
          if (std::filesystem::exists(cd)) {
            // navigate to directory
            std::filesystem::current_path(cd);
          } else if (cd.find("./") == 0 || cd.find("../") == 0) {
            std::string cwd = std::filesystem::current_path();
            std::vector<std::string> paths = split_string(cd, '/');
            try {
              for (size_t i = 0; i < paths.size(); i++) {
                if (paths[i] == ".") {
                  if (std::filesystem::exists(cwd + "/" + paths[i + 1])) {
                    cwd = cwd + "/" + paths[i + 1];
                    i++;
                  } else {
                    throw (cd);
                  }
                } else if (paths[i] == "..") {
                  std::filesystem::path path_obj(cwd);
                  std::filesystem::path parent = path_obj.parent_path();
                  cwd = parent.string();
                } else if (typeid(paths[i]) == typeid(std::string)) {
                  if (std::filesystem::exists(cwd + "/" + paths[i])) {
                    cwd = cwd + "/" + paths[i];
                  } else {
                    throw (cd);
                  }
                }
              }
            }
            catch (std::string cd) {
              std::cout << "cd: " << cd << ": No such file or directory\n";
            }
          } else if (cd.find("~") == 0) {
            std::filesystem::current_path(getenv("HOME"));
          } else {
            std::cout << "cd: " << cd << ": No such file or directory\n";  
          }
        } else {
          // Run external arguments via PATH
          std::vector<std::string> arguments = split_string(input, ' ');
          std::string filepath;
          for(int i = 0; i < path.size(); i++) {
            filepath = path[i] + '/' + arguments[0];
            std::ifstream file(filepath);
            if (file.good()) {
              std::string command = "exec " + path[i] + '/' + input;
              std::system(command.c_str());
              break;
            } else if (i == path.size() - 1) {
              std::cout << input << ": not found\n";
            }
          }
        }
        std::cout << "$ ";
    }

    return 0;
}
