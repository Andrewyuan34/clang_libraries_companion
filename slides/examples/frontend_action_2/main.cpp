#include <format>
#include <map>
#include <string>
#include <string_view>
#include <clang/Basic/LangStandard.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendAction.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
#include <llvm/Support/CommandLine.h>

namespace ct = clang::tooling;

std::string langKindToNameString(clang::LangStandard::Kind kind) {
	return clang::LangStandard::getLangStandardForKind(kind).getName();
}

std::string langKindToLangString(clang::LangStandard::Kind kind) {
	clang::Language language =
	  clang::LangStandard::getLangStandardForKind(kind).getLanguage();
	std::map<clang::Language, std::string> lut{
		{clang::Language::CXX, "C++"},
		{clang::Language::C, "C"},
	};
	auto i = lut.find(language);
	return (i != lut.end()) ? (*i).second : "unknown";
}

//kind是一个枚举类型，它表示了编译器的语言标准，之后getlangStandardForKind(kind)返回一个LangStandard对象，它包含了对应的语言标准，然后通过调用getLanguage()方法，我们可以得到语言类型，如果是C++，则返回CXX，如果是C，则返回C，然后我们通过判断语言类型，返回对应的字符串
std::string langKindToStdString(clang::LangStandard::Kind kind) {
	clang::LangStandard langStd =
	  clang::LangStandard::getLangStandardForKind(kind);
	clang::Language language = langStd.getLanguage();
	if (language == clang::Language::CXX) {
		if (langStd.isCPlusPlus() && !langStd.isCPlusPlus11())
		  {return "C++98";}
		else if (langStd.isCPlusPlus11() && !langStd.isCPlusPlus14())
		  {return "C++11";}
		else if (langStd.isCPlusPlus14() && !langStd.isCPlusPlus17())
		  {return "C++14";}
		else if (langStd.isCPlusPlus17() && !langStd.isCPlusPlus20())
		  {return "C++17";}
		else if (langStd.isCPlusPlus20()) {return "C++20-or-later";}
		else {return "pre-C++98";}
	} else if (language == clang::Language::C) {
		if (langStd.isC99() && !langStd.isC11()) {return "C99";}
		else if (langStd.isC11() && !langStd.isC17()) {return "C11";}
		else if (langStd.isC17()) {return "C17-or-later";}
		else {return "pre-C99";}
	} else {
		return "unknown";
	}
}

// 这里的类继承自SyntaxOnlyAction, which is a kind of ASTConsumer.
class MyFrontendAction : public clang::SyntaxOnlyAction {
	std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
	  clang::CompilerInstance&, llvm::StringRef) override;
};

//这里compInstance是一个CompilerInstance对象，它包含了编译器的所有状态信息, getLangOpts()返回一个LangOptions对象，它包含了编译器的语言选项, Langstd 是langOptions的一个成员变量
std::unique_ptr<clang::ASTConsumer> MyFrontendAction::CreateASTConsumer(
  clang::CompilerInstance& compInstance, llvm::StringRef inFile) {
	const clang::LangOptions& langOpts = compInstance.getLangOpts();
	llvm::outs() << std::format("{}\nlanguage: {}\nstandard: {}\nname: {}\n\n",
	  std::string_view(inFile), langKindToLangString(langOpts.LangStd),
	  langKindToStdString(langOpts.LangStd),
	  langKindToNameString(langOpts.LangStd));
	return clang::SyntaxOnlyAction::CreateASTConsumer(compInstance, inFile);
}

static llvm::cl::OptionCategory toolOptions("Tool Options");

int main(int argc, char** argv) {
	auto expectedOptionsParser = ct::CommonOptionsParser::create(argc,
	  const_cast<const char**>(argv), toolOptions);
	if (!expectedOptionsParser) {
		llvm::errs() << llvm::toString(expectedOptionsParser.takeError());
		return 1;
	}
	ct::CommonOptionsParser& optionsParser = *expectedOptionsParser;
	ct::ClangTool tool(optionsParser.getCompilations(),
	  optionsParser.getSourcePathList());
	int status = tool.run(
	  ct::newFrontendActionFactory<MyFrontendAction>().get());
	if (status) {llvm::errs() << "error detected\n";}
	return !status ? 0 : 1;
}
