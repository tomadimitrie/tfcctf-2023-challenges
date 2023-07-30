use std::{
    io::Write,
    collections::HashMap,
    fmt::Display,
    arch::asm,
    ffi::CString,
    mem::ManuallyDrop,
};

use pest::Parser;
use pest_derive::Parser;
use anyhow::Result;

#[derive(Parser)]
#[grammar  = "lang.pest"]
pub struct LangParser;

#[derive(Debug)]
enum VarType {
    String(String),
    Int(usize)
}

impl VarType {
    fn to_c(&self) -> (usize, Option<ManuallyDrop<CString>>) {
        match self {
            Self::Int(value) => (*value, None),
            Self::String(value) => {
                let c_str = ManuallyDrop::new(CString::new(value.as_str()).unwrap());
                let ptr = c_str.as_ptr();
                (ptr as usize, Some(c_str))
            }
        }
    }
}

impl Display for VarType {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            Self::String(value) => write!(f, "{}", value),
            Self::Int(value) => write!(f, "{}", value)
        }
    }
}

fn process_input(input: &String, vars: &mut HashMap<String, VarType>) {
    match LangParser::parse(Rule::program, &input) {
        Ok(mut result) => {
            let result = result.next().unwrap().into_inner().next().unwrap();
            match result.as_rule() {
                Rule::assignment => {
                    let mut rules = result.into_inner();
                    let var_name = rules.next().unwrap().as_str().to_string();
                    let var_name = var_name.trim();
                    let var_type_rule = rules.next().unwrap().into_inner().next().unwrap();
                    let mut var_type_str = var_type_rule.as_str().to_string();
                    let var_type = match var_type_rule.as_rule() {
                        Rule::string => {
                            var_type_str.pop();
                            var_type_str.remove(0);
                            VarType::String(var_type_str)
                        }
                        Rule::int => {
                            VarType::Int(var_type_str.parse::<usize>().unwrap())
                        }
                        _ => return
                    };
                    println!("Assigned {} to {}", var_name, var_type);
                    vars.insert(var_name.to_string(), var_type);                 
                }
                Rule::print => {
                    let mut rules = result.into_inner();
                    let var_name = rules.next().unwrap().as_str();
                    println!("{}", var_name);
                    if !vars.contains_key(var_name) {
                        println!("No such variable");
                        return;
                    }
                    println!("{}", vars.get(var_name).unwrap())
                }
                Rule::syscall => {
                    let mut rules = result.into_inner();
                    let number = rules.next().unwrap().as_str().parse::<usize>().unwrap();
                    println!("Calling syscall {}", number);
                    let mut valid = true;
                    let args = rules.into_iter().map(|arg| {
                        let result = arg.as_str().to_string();
                        if !vars.contains_key(&result) {
                            println!("No such variable {}", result);
                            valid = false;
                        }
                        result
                    }).collect::<Vec<_>>();
                    if !valid {
                        return;
                    }
                    let args = args.iter().map(|arg| vars.get(arg).unwrap().to_c()).collect::<Vec<_>>();
                    println!("{:?}", args);
                    let rdi = if args.len() >= 1 {
                        args[0].0
                    } else { 0 };
                    let rsi = if args.len() >= 2 {
                        args[1].0
                    } else { 0 };
                    let rdx = if args.len() >= 3 {
                        args[2].0
                    } else { 0 };
                    let r10 = if args.len() >= 4 {
                        args[3].0
                    } else { 0 };
                    let r8 = if args.len() >= 5 {
                        args[4].0
                    } else { 0 };
                    let r9 = if args.len() >= 6 {
                        args[5].0
                    } else { 0 };

                    unsafe {
                        asm!(
                            // "int3",
                            "syscall",

                            in("rax") number,
                            in("rdi") rdi,
                            in("rsi") rsi,
                            in("rdx") rdx,
                            in("r10") r10,
                            in("r8") r8,
                            in("r9") r9
                        );
                    }

                    for arg in args {
                        if let Some(mut arg) = arg.1 {
                            unsafe {
                                ManuallyDrop::drop(&mut arg);
                            }
                        }
                    }
                }
                _ => ()
            }
        }
        Err(error) => {
            println!("{:?}", error);
        }
    }
}

fn main() -> Result<()> {
    let mut vars: HashMap<String, VarType> = HashMap::new();
    loop {
        print!("> ");
        std::io::stdout().flush()?;
        let mut input = String::new();
        std::io::stdin().read_line(&mut input)?;

        process_input(&input, &mut vars);
    }
}
