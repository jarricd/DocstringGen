#pragma once
#include <iostream>
#include <llama.h>
#include <string>
#include <vector>
#include "Tools.h"
#include "args.hxx"

namespace generator {
    std::string generate_docstring(std::string target_path)
    {
        std::string model_path = "K:\\models\\qwen-coder-14b\\converted\\qwen_coder_14b-Q4_K_M.gguf";
        std::cout << "Used model: " << model_path << std::endl;
        std::string system_prompt = tools::load_file("K:\\DocstringGenerator\\DocstringGen\\x64\\Debug\\chat_template.jinja2"); //dev only
        std::string python_fun[] = { target_path };

        //todo: config
        const int ngl = 32;
        const int n_ctx = 10240 * 1;

        // only print errors
        llama_log_set([](enum ggml_log_level level, const char* text, void* /* user_data */) {
            if (level >= GGML_LOG_LEVEL_ERROR) {
                fprintf(stderr, "%s", text);
            }
            }, nullptr);

        // load dynamic backends
        ggml_backend_load_all();

        // initialize the model
        llama_model_params model_params = llama_model_default_params();
        model_params.n_gpu_layers = ngl;

        llama_model* model = llama_model_load_from_file(model_path.c_str(), model_params);
        if (!model) {
            fprintf(stderr, "%s: error: unable to load model\n", __func__);
            return "";
        }

        const llama_vocab* vocab = llama_model_get_vocab(model);

        // initialize the context
        llama_context_params ctx_params = llama_context_default_params();
        ctx_params.n_ctx = n_ctx;
        ctx_params.n_batch = 512;
        ctx_params.n_threads = 12;
        ctx_params.n_threads_batch = 12;

        llama_context* ctx = llama_init_from_model(model, ctx_params);
        if (!ctx) {
            fprintf(stderr, "%s: error: failed to create the llama_context\n", __func__);
            return "";
        }

        // initialize the sampler
        llama_sampler* smpl = llama_sampler_chain_init(llama_sampler_chain_default_params());
        //llama_sampler_chain_add(smpl, llama_sampler_init_top_k(50));
        llama_sampler_chain_add(smpl, llama_sampler_init_top_p(0.5f, 2));
        llama_sampler_chain_add(smpl, llama_sampler_init_temp(0.5f));
        llama_sampler_chain_add(smpl, llama_sampler_init_dist(2137));


        std::vector<llama_chat_message> messages;
        std::vector<char> formatted(llama_n_ctx(ctx));
        std::string response = "";
        
        int prev_len = 0;
 
        
        for (std::string target_fun : python_fun) {
            
            std::string function_content = tools::load_file(target_fun);
            if (function_content.length() == 0)
            {
                std::cerr << "Read file is empty." << std::endl;
                return "";
            }

            std::string test_prompt_str = "Generate a Numpy style docstring for the provided functions: \n" + function_content + "\n Numpy-doc style does not include colons at the end of sections. Output only the generated docstring with the function. Ensure that the generated docstring is conforming to the Numpy-doc style.";

            // get user input
            std::cout << "Prompt: " << test_prompt_str << std::endl;

            const char* tmpl = system_prompt.c_str();

            // add the user input to the message list and format it
            messages.push_back({ "user", _strdup(test_prompt_str.c_str()) });
            int new_len = llama_chat_apply_template(tmpl, messages.data(), messages.size(), true, formatted.data(), formatted.size());
            if (new_len > (int)formatted.size()) {
                formatted.resize(new_len);
                new_len = llama_chat_apply_template(tmpl, messages.data(), messages.size(), true, formatted.data(), formatted.size());
            }
            if (new_len < 0) {
                fprintf(stderr, "failed to apply the chat template\n");
                return "";
            }

            // remove previous messages to obtain the prompt to generate the response
            std::string prompt(formatted.begin() + prev_len, formatted.begin() + new_len);
            response = tools::generate(prompt, ctx, vocab, smpl);

            
            // add the response to the messages
            messages.push_back({ "assistant", _strdup(response.c_str()) });
            prev_len = llama_chat_apply_template(tmpl, messages.data(), messages.size(), false, nullptr, 0);
            if (prev_len < 0) {
                fprintf(stderr, "failed to apply the chat template\n");
                return "";
            }
        }
        // free resources
        for (auto& msg : messages) {
            free(const_cast<char*>(msg.content));
        }
        llama_sampler_free(smpl);
        llama_free(ctx);
        llama_model_free(model);

        return response;
    }

}