import os
import json
import argparse
from evaler import APIEvaler, ChatEvaler,CosecEvaler
from tqdm import tqdm

os.environ["ANTHROPIC_API_KEY"] = ""
os.environ["OPENAI_API_KEY"] = ""
os.environ["GOOGLE_API_KEY"] = ""  

def main(args):
    # Choose the appropriate evaler based on the command-line argument
    if args.evaler == 'cosec':
        evaler = CosecEvaler(
            args.model_name,
            args.final_model,
            args.context_type,
            args.prompt_type,
            args.mode, args
        )
    
    elif "DeepSeek-" in args.model_name or 'gpt-' in args.model_name or 'claude-' in args.model_name or 'gemini-' in args.model_name:
        evaler = APIEvaler(args.model_name, args.context_type, args.prompt_type, args.mode)
    else:
        evaler = ChatEvaler(args.model_name, args.context_type, args.prompt_type, args.mode)

    with open('ids.txt', 'r') as f:
        ids = f.read().splitlines()[1:]
    res = []

    for id in tqdm(ids):
        # print(id)
        
        response, prompt, system_prompt = evaler.get_response(id, args.mode)
        
        if prompt == None or system_prompt == None:
            print(f"Prompt or system prompt is None for ID: {id}")

            continue
        
        res.append(response)
        if not os.path.exists(f'cosec_completions/{id}'):
            os.makedirs(f'cosec_completions/{id}')
        with open(f'cosec_completions/{id}/{args.model_name}-filled-code-{args.context_type}-{args.prompt_type}-{args.mode}_code_completion.txt', 'w') as file:
            file.write(response)
        with open(f'cosec_completions/{id}/{args.model_name}-filled-code-{args.context_type}-{args.prompt_type}-{args.mode}_prompt.txt', 'w') as file:
            file.write(prompt)
        with open(f'cosec_completions/{id}/{args.model_name}-filled-code-{args.context_type}-{args.prompt_type}-{args.mode}_system_prompt.txt', 'w') as file:
            file.write(system_prompt)
        evaler.save_cache()
    evaler.save_cache()
    import ipdb; ipdb.set_trace()

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--model_name', type=str, default='deepseek-ai/deepseek-coder-1.3b-instruct')
    parser.add_argument('--context_type', type=str, default='cross-file')
    parser.add_argument('--prompt_type', type=str, default='system-prompt')
    parser.add_argument('--mode', type=str, default='perturbed')
    parser.add_argument('--evaler', type=str, default='chat', choices=['api', 'chat', 'cosec'])
    # Arguments for CosecEvaler (only used when --evaler cosec)
    parser.add_argument('--final_model', type=str, default='/space1/pchiniya/CoSec/sven/trained/deepseek/final_merged')
    parser.add_argument('--temp', type=float, default=0.4)
    parser.add_argument('--top_p', type=float, default=0.95)
    parser.add_argument('--max_gen_len', type=int, default=256)
    parser.add_argument('--exp_temp', type=float, default=0.4)
    parser.add_argument('--threshold', type=float, default=0.3)
    args = parser.parse_args()
    main(args)