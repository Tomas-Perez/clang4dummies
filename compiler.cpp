#include <iostream>
#include <sys/stat.h>
#include <vector>
#include <string>

#include "clang/Driver/Driver.h"
#include "clang/Driver/Compilation.h"
#include "clang/Driver/Job.h"
#include "clang/Frontend/CompilerInvocation.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/CodeGen/CodeGenAction.h"
#include "clang/Basic/DiagnosticIDs.h"
#include "clang/Basic/DiagnosticOptions.h"

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Target/TargetMachine.h"

#include "LLVMInstanceManager.hpp"

bool exists(const std::string &name)
{
  struct stat buffer;
  return (stat(name.c_str(), &buffer) == 0);
}

bool generateBin(const std::vector<std::string> &cmds, const std::string outputFilename)
{
  auto _llvmManager = LLVMInstanceManager::getInstance();

  auto _diagnosticOptions = new clang::DiagnosticOptions();
  auto _diagnosticIDs = new clang::DiagnosticIDs();
  auto _diagEngine = new clang::DiagnosticsEngine(_diagnosticIDs, _diagnosticOptions);

  std::vector<const char *> cmd_str;

  for (const auto& cmd : cmds) {
    cmd_str.push_back(cmd.c_str());
  }

  const std::string outputArgument = "-o" + outputFilename;
  cmd_str.push_back(std::move(outputArgument).c_str());

  // log the command line string used to create this task
  std::string log_str = "";
  for (const auto& arg : cmd_str) {
    std::cout << arg << " ";
  }
  std::cout << std::endl;

  clang::driver::Driver NikiLauda(_llvmManager->getClangExePath(),
                           _llvmManager->getDefaultTriple()->str(),
                           *_diagEngine);
  NikiLauda.setCheckInputsExist(false);
  NikiLauda.CCPrintOptionsFilename = "logging.log";
  NikiLauda.CCPrintOptions = true;

  std::unique_ptr<clang::driver::Compilation> C(NikiLauda.BuildCompilation(cmd_str));
  if (!C) {
    std::cerr << "clang::driver::Compilation not created" << std::endl;
    return false;
  }

  llvm::SmallVector<std::pair<int, const clang::driver::Command*>,1> failCmd;  
  const auto res = NikiLauda.ExecuteCompilation(*C, failCmd);

  if (exists(outputFilename)) {
    return true;
  }

  std::cerr 
    << "Unknown error:"
    << " unable to generate shared object"
    << " - Driver error code: " 
    << std::to_string(res) 
    << std::endl;

  return false;
}

int main(void)
{
	// Commands to run for compilation
	std::vector<std::string> cmds = {"clang" , "compile_me.c"};

  generateBin(cmds, "output");
}
