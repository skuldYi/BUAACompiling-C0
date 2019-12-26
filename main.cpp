#include "argparse.hpp"
#include "fmt/core.h"

#include "tokenizer/tokenizer.h"
#include "analyser/analyser.h"
#include "generater/generator.h"
#include "binary/binary.h"
#include "fmts.hpp"

#include <iostream>
#include <string>

std::vector<c0::Token> _tokenize(std::istream& input) {
	c0::Tokenizer tkz(input);
	auto p = tkz.AllTokens();
	if (p.second.has_value()) {
		fmt::print(stderr, "Tokenization error: {}\n", p.second.value());
		exit(2);
	}
	return p.first;
}

void Tokenize(std::istream& input, std::ostream& output) {
	auto v = _tokenize(input);
	for (auto& it : v)
		output << fmt::format("{}\n", it);
	return;
}

void Analyse(std::istream& input, std::ostream& output){
	auto tks = _tokenize(input);
	c0::Analyser analyser(tks);
	auto p = analyser.Analyse();
	if (p.second.has_value()) {
		fmt::print(stderr, "Syntactic analysis error: {}\n", p.second.value());
		exit(2);
	}
	auto v = p.first;
	for (auto& it : v)
		output << fmt::format("{}\n", it);
}

void Compile(std::istream& input, std::ostream& output){
	auto tks = _tokenize(input);
	c0::Analyser analyser(tks);
	auto ana = analyser.Analyse();
	if (ana.second.has_value()) {
		fmt::print(stderr, "Syntactic analysis error: {}\n", ana.second.value());
		exit(2);
	}
	auto quad = ana.first;
    c0::Generator generator(quad);
    auto code = generator.Generate();

    int i;
    output << ".constants:\n";
    i = 0;
    for (const auto& pair : code.constants) {
        output << i++ << "\t" << pair.first << "\t\"" << pair.second << "\"\n";
    }

    output << ".start:\n";
    i = 0;
    for (const auto& it : code.start) {
        output << i++ << "\t" << fmt::format("{}\n", it);
    }

    output << ".functions:\n";
    i = 0;
    for (const auto& it : code.functions) {
        output << i++ << "\t" << fmt::format("{}\n", it);
    }

    for (i = 0; i < (int)code.instructions.size(); i++) {
        output << ".F" << i << ":\n";
        int j = 0;
        for (const auto &it : code.instructions[i]) {
            output << j++ << "\t" << fmt::format("{}\n", it);
        }
    }
}

void BinaryCode(std::istream& input, std::ofstream& output){
	auto tks = _tokenize(input);
	c0::Analyser analyser(tks);
	auto ana = analyser.Analyse();
	if (ana.second.has_value()) {
		fmt::print(stderr, "Syntactic analysis error: {}\n", ana.second.value());
		exit(2);
	}
	auto quad = ana.first;
    c0::Generator generator(quad);
    auto code = generator.Generate();

    c0::Binary binary(code.constants, code.start, code.functions, code.instructions);
    binary.output_binary(output);
}

int main(int argc, char** argv) {
	argparse::ArgumentParser program("c0");
	program.add_argument("input")
		.help("speicify the file to be compiled.");
//	program.add_argument("-t")
//		.default_value(false)
//		.implicit_value(true)
//		.help("perform tokenization for the input file.");
	program.add_argument("-l")
		.default_value(false)
		.implicit_value(true)
		.help("perform syntactic analysis for the input file.");
	program.add_argument("-s")
		.default_value(false)
		.implicit_value(true)
		.help("generate byte code for the input file.");
	program.add_argument("-c")
		.default_value(false)
		.implicit_value(true)
		.help("generate binary object file for the input file.");
	program.add_argument("-o", "--output")
		.required()
		.default_value(std::string("-"))
		.help("specify the output file.");

	try {
		program.parse_args(argc, argv);
	}
	catch (const std::runtime_error& err) {
		fmt::print(stderr, "{}\n\n", err.what());
		program.print_help();
		exit(2);
	}

	auto input_file = program.get<std::string>("input");
	auto output_file = program.get<std::string>("--output");
	std::istream* input;
	std::ostream* output;
	std::ifstream inf;
	std::ofstream outf;
	if (input_file == "-") {
        fmt::print(stderr, "Input file is required.\n");
        exit(2);
    }

    inf.open(input_file, std::ios::in);
    if (!inf) {
        fmt::print(stderr, "Fail to open {} for reading.\n", input_file);
        exit(2);
    }
    input = &inf;

	if (output_file == "-")
	    output_file = "out";

	if (program["-c"] == true)
        outf.open(output_file, std::ios::binary | std::ios::out | std::ios::trunc);
	else
        outf.open(output_file, std::ios::out | std::ios::trunc);

    if (!outf) {
        fmt::print(stderr, "Fail to open {} for writing.\n", output_file);
        exit(2);
    }
    output = &outf;

	if (program["-s"] == true && program["-c"] == true) {
		fmt::print(stderr, "You can only generate byte code or binary file at one time.");
		exit(2);
	}

//	if (program["-t"] == true) {
//		Tokenize(*input, *output);
//	}
	if (program["-l"] == true) {
		Analyse(*input, *output);
	}
	else if (program["-s"] == true) {
		Compile(*input, *output);
	}
	else if (program["-c"] == true) {
        BinaryCode(*input, outf);
	}
	else {
		fmt::print(stderr, "You must choose  byte code or binary file to generate.");
		exit(2);
	}
	return 0;
}