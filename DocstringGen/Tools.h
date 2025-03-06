#pragma once
#define PY_SSIZE_T_CLEAN

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <Python.h>

namespace tools {

    int write_file(std::string content, std::string output_path)
    {
        std::cout << "Writing file to path: " << output_path << std::endl;
        std::ofstream file_handle(output_path);

        if (!file_handle.is_open())
        {
            std::cerr << "Template load failed" << std::endl;
            return -1;
        }

        file_handle << content;
        file_handle.close();
        return 0;
    }

    std::string load_file(std::string target_path)
    {
        std::cout << "Loading file from path " << target_path << std::endl;
        std::string cur_cwd = std::filesystem::current_path().string();
        std::string full_path = cur_cwd + "\\" + target_path;
        std::ifstream template_handle(target_path);
        if (!template_handle.is_open()) {

            char msg_buf[4096] = { 0 };
            strerror_s(msg_buf, 4096, errno);
            throw std::runtime_error("Could not open file: " + full_path + ". Error: " + std::string(msg_buf));
        }

        try {
            // Use a stringstream to read the entire file content
            std::stringstream buffer;
            buffer << template_handle.rdbuf();

            return buffer.str();
        }
        catch (const std::exception& e) {
            // Handle exceptions that may occur during the read process
            std::cerr << "Exception while reading file: " << e.what() << std::endl;
            throw;  // Re-throw the exception if necessary
        }
    }

    // helper function to evaluate a prompt and generate a response
    std::string generate(const std::string& prompt, llama_context *ctx, const llama_vocab *vocab, llama_sampler *smpl) {
        std::string response = "";

        const bool is_first = llama_get_kv_cache_used_cells(ctx) == 0;

        // tokenize the prompt
        const int n_prompt_tokens = -llama_tokenize(vocab, prompt.c_str(), prompt.size(), NULL, 0, is_first, true);
        std::vector<llama_token> prompt_tokens(n_prompt_tokens);
        if (llama_tokenize(vocab, prompt.c_str(), prompt.size(), prompt_tokens.data(), prompt_tokens.size(), is_first, true) < 0) {
            GGML_ABORT("failed to tokenize the prompt\n");
        }

        // prepare a batch for the prompt
        llama_batch batch = llama_batch_get_one(prompt_tokens.data(), prompt_tokens.size());
        llama_token new_token_id;
        while (true) {
            // check if we have enough space in the context to evaluate this batch
            int n_ctx = llama_n_ctx(ctx);
            int n_ctx_used = llama_get_kv_cache_used_cells(ctx);
            if (n_ctx_used + batch.n_tokens > n_ctx) {
                printf("\033[0m\n");
                fprintf(stderr, "context size exceeded\n");
                exit(0);
            }

            if (llama_decode(ctx, batch)) {
                GGML_ABORT("failed to decode\n");
            }

            // sample the next token
            new_token_id = llama_sampler_sample(smpl, ctx, -1);

            // is it an end of generation?
            if (llama_vocab_is_eog(vocab, new_token_id)) {
                break;
            }

            // convert the token to a string, print it and add it to the response
            char buf[256];
            int n = llama_token_to_piece(vocab, new_token_id, buf, sizeof(buf), 0, true);
            if (n < 0) {
                GGML_ABORT("failed to convert token to piece\n");
            }
            std::string piece(buf, n);
            printf("%s", piece.c_str());
            fflush(stdout);
            response += piece;

            // prepare the next batch with the sampled token
            batch = llama_batch_get_one(&new_token_id, 1);
        }

        return response;
        };


    int execute_python_file(std::string target_path, char **argv, std::string *python_result)
    {
        std::string script_content = load_file(target_path);

        PyStatus status;
        PyConfig config;
        PyConfig_InitPythonConfig(&config);

        status = PyConfig_SetBytesString(&config, &config.program_name, argv[0]);
        if (PyStatus_Exception(status)) {
            PyConfig_Clear(&config);
            Py_ExitStatusException(status);
            return 1;
        }

        status = Py_InitializeFromConfig(&config);
        if (PyStatus_Exception(status)) {
            PyConfig_Clear(&config);
            Py_ExitStatusException(status);
            return 2;
        }
        PyConfig_Clear(&config);
        auto output = PyRun_SimpleString(script_content.c_str());

        if (Py_FinalizeEx() < 0) {
            return -1;
        }

        PyConfig_Clear(&config);
    }
}