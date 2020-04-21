#include "clang/Driver/Driver.h"
#include "clang/Driver/Compilation.h"
#include "clang/Driver/Job.h"
#include "clang/Frontend/CompilerInvocation.h"
#include "clang/CodeGen/CodeGenAction.h"
#include <iostream>
#include <sys/stat.h>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "clang/Basic/DiagnosticIDs.h"
#include "clang/Basic/DiagnosticOptions.h"
#include "clang/Frontend/CompilerInstance.h"

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Target/TargetMachine.h"


#include "LLVMInstanceManager.hpp"
// This requires another library I could not find, but is just used to log output to a file.
// A log is being generated anyway, so I'm not sure what is going on.
// #include "FileLogDiagnosticConsumer.hpp"

using namespace clang;

bool exists(const std::string &name)
{
  struct stat buffer;
  return (stat(name.c_str(), &buffer) == 0);
}

std::string generateBin(const std::vector<std::string> &src)
{
    // initialize LLVM stuff and shutdown everything at program tear down
  auto _llvmManager = LLVMInstanceManager::getInstance();

  // custom diagnostic engine creation
  auto _diagnosticOptions = new clang::DiagnosticOptions();
  auto _diagnosticIDs = new clang::DiagnosticIDs();
  // auto _diagConsumer = std::make_shared<vc::FileLogDiagnosticConsumer>(
  //                                      "logging.log",
  //                                      _diagnosticOptions);
  auto _diagEngine = new clang::DiagnosticsEngine(_diagnosticIDs,
                                             _diagnosticOptions
                                            //  _diagConsumer.get(),
                                            //  false
                                            );


  // What we want to generate
  const std::string libFileName = "output";
  // what we return when generateIR fails
  const std::string failureFileName = "";
  std::string log_str = "";

  auto report_error = [&](const std::string message) {
    std::string error_string = "ClangLibCompiler::generateBin";
    error_string = error_string + " ERROR during processing of version ";
    error_string = error_string + "1.0";
    error_string = error_string + "\n\t";
    error_string = error_string + message;
    std::cout << error_string << std::endl;
    return;
  };

  // clang++ <options> -fpic -shared src -olibFileName -Wno-return-type-c-linkage
  std::vector<const char *> cmd_str;
  // cmd_str.reserve(options.size() + 6);
  cmd_str.push_back(std::move("clang++"));
  cmd_str.push_back(std::move("-fpic"));
  // cmd_str.push_back(std::move("-shared"));
  cmd_str.push_back(std::move("-Wno-return-type-c-linkage"));
  const std::string outputArgument = "-o" + libFileName;
  cmd_str.push_back(std::move(outputArgument).c_str());

  // create a local copy of option strings
  // const auto& argv_owner = getArgV(options);
  // std::vector<const char*> argv;
  // argv.reserve(argv_owner.size());
  // for (const auto& arg : argv_owner) {
  //   argv.push_back(arg.c_str());
  // }
  // cmd_str.insert(cmd_str.end(),
  //                argv.begin(),
  //                argv.end());
  for (const auto& src_file : src) {
    cmd_str.push_back(src_file.c_str());
  }

  // log the command line string used to create this task
  for (const auto& arg : cmd_str) {
    log_str = log_str + arg + " ";
  }

  driver::Driver NikiLauda(_llvmManager->getClangExePath(),
                           _llvmManager->getDefaultTriple()->str(),
                           *_diagEngine);
  NikiLauda.setTitle("clang as a library");
  NikiLauda.setCheckInputsExist(false);
  NikiLauda.CCPrintOptionsFilename = "logging.log";
  NikiLauda.CCPrintOptions = true;

  std::unique_ptr<driver::Compilation> C(NikiLauda.BuildCompilation(cmd_str));
  if (!C) {
    report_error("clang::driver::Compilation not created");
    return failureFileName;
  }

  llvm::SmallVector<std::pair<int, const driver::Command*>,1> failCmd;
  const auto res = NikiLauda.ExecuteCompilation(*C, failCmd);

  if (exists(libFileName)) {
    return libFileName;
  }
  const std::string &error_str = "Unknown error:"
                                " unable to generate shared object"
                                " - Driver error code: " + std::to_string(res);
  report_error(error_str);
  return failureFileName;
}

int main(void)
{
	// Path to the C file
	std::vector<std::string> src = {"compile_me.c"};

  generateBin(src);
}
