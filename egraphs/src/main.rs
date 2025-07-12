#![allow(warnings)]
extern crate clap;
use clap::{App, Arg};
use egraphslib::*;
use std::time::Instant; 
use crate::veclang::VecLang;
use egg::{RecExpr, Id, Language};
use rand::Rng;
use std::{env, fs};
/************************************/
use std::collections::BinaryHeap;
use std::cmp::Ordering; 
use std::cmp::Reverse; 

#[derive(Debug, Eq, PartialEq)]
struct State {
    cost: usize,
    max_vector_width : usize ,
    expression: RecExpr<VecLang>, // Additional data (e.g., node ID)
}

// Implement Ord for custom sorting based on cost
impl Ord for State {
    fn cmp(&self, other: &Self) -> Ordering {
        other.cost.cmp(&self.cost) // Min-heap: lower cost has higher priority
    }
}

// Implement PartialOrd to be consistent with Ord
impl PartialOrd for State {
    fn partial_cmp(&self, other: &Self) -> Option<Ordering> {
        Some(self.cmp(other))
    }
}

/************************************/
fn main() { 
    let matches = App::new("Rewriter")
    .arg(
        Arg::with_name("INPUT")
            .help("Sets the input file") 
            .required(true)
            .index(1), 
    ) 
    .arg(
        Arg::with_name("vector_width")
            .help("Sets the vector_width")
            .required(true)
            .index(2),
    ) 
    .get_matches();
    // Get a path string to parse a program.
    let path = matches.value_of("INPUT").unwrap();
    let timeout = env::var("TIMEOUT")
        .ok()
        .and_then(|t| t.parse::<u64>().ok())
        .unwrap_or(300);
    let prog_str = fs::read_to_string(path).expect("Failed to read the input file.");
    let mut prog : RecExpr<VecLang>= prog_str.parse().unwrap();
    let vector_width: usize = matches
        .value_of("vector_width")
        .unwrap() 
        .parse()
        .expect("Number must be a valid usize");
    /*********************************/
    /*********************************/
    let start_time = Instant::now();
    // Run rewriter
    eprintln!(
        "Running egg with timeout {:?}s, width: {:?}",
        timeout, vector_width
    );
    let mut current_expr = prog.clone();
    let mut stop_reason = 0 ;
    let mut node_limit = 100_000 ;
    /*********************************************/ 
    /*********************************************/ 
    let mut current_vector_width = vector_width ; 
    let (cost, best, stop_reason) = rules::run(&current_expr, timeout, current_vector_width,node_limit,0,0);
    let best_expr = best ; 
    current_vector_width = rules::get_vector_width(&best_expr);
    let best_cost = cost ;
    eprintln!("Optimized expression cost : {} ", best_cost);
    eprintln!("Obtained expression ==> : {}", best_expr.to_string());
    /*****************************************************************************
    rulesets_appplying_order  = vec![9,10,11,12,13,14,15];
    let mut best_depth = rules::ast_depth(&best_expr);
    for iteration in 0..24 {
        let (cost, best , stop_reason) = rules::run(&current_expr, timeout, vector_width, node_limit,rulesets_appplying_order[iteration%rulesets_appplying_order.len()],0);
        let depth = rules::ast_depth(&best);
        if depth < best_depth {
            best_depth=depth ;  
            current_expr = best ; 
            best_expr = current_expr.clone() ;
        }
        eprintln!("Best cost at iteration {}: {} , Depth {} \n\n", iteration + 1, cost,depth);
    }
    *****************************************************************************/
    let duration = start_time.elapsed();
    println!("{}", best_expr.to_string()); /* Pretty print with width 80 */
    println!("{} {}",current_vector_width,current_vector_width);
}
