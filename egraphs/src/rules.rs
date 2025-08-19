use std::usize;

use crate::{
    extractor::GreedyExtractor, 
    extractor_sa::SimulatedAnnealingExtractor,
    extractor_exhaustive::ExhaustiveExtractor, 
    veclang::{ConstantFold, Egraph, VecLang},
    runner::Runner,
    runner::StopReason, 
    cost::VecCostFn, 
}; 
use std::collections::VecDeque;
use std::collections::HashMap;   
use std::collections::HashSet;  
use egg::rewrite as rw;
use egg::*;
use core::cmp::min;
// Check if all the variables, in this case memories, are equivalent

/// Run the rewrite rules over the input program and return the best (cost, program)
use std::time::Instant; 

pub fn run(
    prog: &RecExpr<VecLang>,
    timeout: u64,
    vector_width: usize,
    node_limit: usize,
    selected_ruleset_order : usize ,
    extraction_technic : usize ,
    bounded_expr : &mut bool,
) -> (usize, RecExpr<VecLang>, usize) {
    // Initialize the rule set based on the vector width
    let mut rules: Vec<Rewrite<VecLang, ConstantFold>> = vec![];
    // Find the depth of the expression 
    let expression_depth : usize = ast_depth(&prog);
    match selected_ruleset_order {
        1 => {
            if !(*bounded_expr) {
                eprintln!("new rules");
                // rules.extend(generate_associativity_rules(128));
                // rules.extend(generate_advanced_rules(128));
                *bounded_expr = true;
            }
        },
        //1 => {rules = vector_rules(vector_width);},
        2 => {
            rules = addition_rules(vector_width,expression_depth);},
        3 => {rules = minus_rules(vector_width,expression_depth);},
        4 => {rules = multiplication_rules(vector_width,expression_depth);},
        5 => {rules = neg_rules(vector_width,expression_depth); }, 

        //7 => {rules = rot_minus_rules(vector_width,expression_depth);},
        //8 => {rules = rot_multiplication_rules(vector_width,expression_depth);},

        9 => {rules = vector_assoc_add_rules(vector_width);},
        10 => {rules = vector_assoc_min_rules(vector_width);},
        11 => {rules = vector_assoc_mul_rules(vector_width);},
        12 => {rules = vector_assoc_add_mul_rules(vector_width);},
        13 => {rules = vector_assoc_add_min_rules(vector_width);},
        14 => {rules = vector_assoc_min_mul_rules(vector_width);},
        15 => {rules = assoc_neg_rules(vector_width);},
        _ => eprintln!("Ruleset correspoding to this order doesnt exist"),
    }
    let mut iteration_count = 0;
    // Start timing the e-graph building process
    let start_time = Instant::now();
    // Initialize the e-graph with constant folding enabled and add a zero literal
    let mut init_eg = Egraph::new(ConstantFold);
    init_eg.add(VecLang::Num(0));
    type MyRunner = Runner<VecLang, ConstantFold>;
    let runner = MyRunner::new(Default::default())
        .with_egraph(init_eg)
        .with_expr(&prog)
        .with_node_limit(node_limit)
        .with_time_limit(std::time::Duration::from_secs(timeout))
        .with_iter_limit(10_000)
        .with_hook({
            move |runner| {
                /* print the egraph after each iteration */
                //  eprintln!("egraph in iteration : {:} ***********", iteration_count);
                // print_egraph(runner.egraph.clone());
                Ok(())
            }
        })
        .run(&rules);
        let report = runner.report();
        //eprintln!("report : {:?}", report);
        /* for the rules , if the rule is expensive we add the prefix exp to its name */
    // Stop timing after the e-graph is built
    let build_time = start_time.elapsed();
    
    //eprintln!("E-graph built in {:?}", build_time);
    // Print the reason for stopping to STDERR
    
    /*eprintln!(
        "Stopped after {} iterations, reason: {:?}",
        runner.iterations.len(),
        runner.stop_reason
    );*/
    
    // Extract the e-graph and the root node
    let (eg, root) = (runner.egraph, runner.roots[0]);
    //eprintln!("final number of enodes : {:?}", eg.total_size());
    let find_cycle = Instant::now();
    find_cycles(&eg);
    let time_end_cycles = find_cycle.elapsed();
    
    //eprintln!("time for finding cyclse is : {:?}", time_end_cycles);
    /*
    Stop_reason : 0  : Not-saturated 
                  1  : Saturated 
    */
    
    let mut stop_reason : usize = 0 ;
    match runner.stop_reason {
        Some(StopReason::Saturated) => {
               stop_reason=1;
        }
        _ => {stop_reason = 0;}, 
    }
    /********************************************************************/
    let mut best_cost = usize::MAX;
    let mut best_expr: RecExpr<VecLang> = RecExpr::default();
    //eprintln!("begining of extraction 0 .... ");
    /* we have 3 ways fot the extraction:
        1) greedy_extraction: takes decisions locally
        2) exhaustive_extraction: exploring all possibilities
        3) sa_extraction: based on simulating annealing metaheuristic
    */
    /*********************** greedy extraction **************************/
    /********************************************************************/
    if extraction_technic == 0 {
        let start_extract_time = Instant::now();
        let mut extractor = GreedyExtractor::new(&eg, VecCostFn { egraph: &eg }, root);
        (best_cost, best_expr) = extractor.find_best(root);
        let extract_time = start_extract_time.elapsed();
    }else if extraction_technic == 1 {
        /******************************* Exhaustive extraction ************************/
        /******************************************************************************/
        let start_extract_time = Instant::now();
        let mut extractor = ExhaustiveExtractor::new(&eg);
        extractor.find_best(
            vec![root],
            HashMap::new(),
            root,
            0,
            0,
            vec![],
            &mut best_cost,
            &mut best_expr,
            &mut HashMap::new(), 
        );
        let extract_time = start_extract_time.elapsed();
        /******************************************************************************/
    }else if extraction_technic == 2 {
        /********************************** SA extraction *****************************/
        /******************************************************************************/
        let start_extract_time = Instant::now();
        let mut extractor = SimulatedAnnealingExtractor::new(&eg);
        let mut n_cost:usize = 0;
        // // Parameters for simulated annealing
        let max_iteration = 200000;
        let initial_temp = 200000.0;
        let cooling_rate = 0.995;
        (best_cost, best_expr) = extractor.find_best(
            &eg,
            root,
            max_iteration,
            initial_temp,
            cooling_rate,
        );
        let extract_time = start_extract_time.elapsed();
    }
    (best_cost, best_expr, stop_reason)

}
/***********************************************************************************/
/***********************************************************************************/
/***********************************************************************************/
pub fn ast_depth(expr: &RecExpr<VecLang>) -> usize {
    fn depth_helper(id: Id, expr: &RecExpr<VecLang>) -> usize {
        let node = &expr[id];
        if node.children().is_empty() {
            0 // Leaf node, depth is 1
        } else { 
            // Depth is 1 + max depth of all child nodes
            1 + node.children().iter().map(|&child| depth_helper(child, expr)).max().unwrap_or(0)
        }
    }
    // Start with the root node (usually the last node in `RecExpr`)
    depth_helper(Id::from(expr.as_ref().len() - 1), expr)
}
/************************************/
pub fn get_vector_width(expr: &RecExpr<VecLang>) -> usize {
    fn vector_width_helper(id: Id, expr: &RecExpr<VecLang>) -> usize {
        let node = &expr[id];
        if matches!(node, VecLang::Vec(_)){
            node.children().len()
        }else{
            node.children().iter().map(|&child| vector_width_helper(child, expr)).max().unwrap_or(0)
        }
    }
    // Start with the root node (usually the last node in `RecExpr`)
    vector_width_helper(Id::from(expr.as_ref().len() - 1), expr)
}
/***********************************/
fn find_cycles<L, N>(egraph: &EGraph<L, N>) -> Vec<(Id, usize)>
where
    L: Language,
    N: Analysis<L>,
{
    enum Color {
        White,
        Gray,
        Black,
    }
    type Enter = bool;

    // Structure to store the detected cycles
    let mut cycles: Vec<(Id, usize)> = Vec::new();

    // Color map to track the state of each e-class
    let mut color: HashMap<Id, Color> = egraph.classes().map(|c| (c.id, Color::White)).collect();

    // Stack for the depth-first search
    let mut stack: Vec<(Enter, Id)> = egraph.classes().map(|c| (true, c.id)).collect();

    // Traverse the e-graph
    while let Some((enter, id)) = stack.pop() {
        if enter {
            *color.get_mut(&id).unwrap() = Color::Gray;
            stack.push((false, id));  // Mark the node for completion after exploring its children
            
            // Iterate over each node in the e-class
            for (i, node) in egraph[id].iter().enumerate() {
                // Iterate over children of the node
                for child in node.children() {
                    match &color[child] {
                        Color::White => stack.push((true, *child)),  // Traverse deeper if unvisited
                        Color::Gray => cycles.push((id, i)),  // Cycle detected, push to the result
                        Color::Black => (),  // Already fully explored
                    }
                }
            }
        } else {
            *color.get_mut(&id).unwrap() = Color::Black;  // Mark node as fully processed
        }
    }

    //eprintln!("cycles are : {:?}", cycles);

    // Return the detected cycles
    cycles
}
/*************************************/
pub fn print_egraph(egraph: Egraph)
{

    eprintln!("***************egraph******************");

                for eclass in egraph.classes() {
                    // Print the e-class ID
                    eprint!("E-Class {{Id: {}}} =", eclass.id);
            
                    // Iterate over all enodes in the e-class and print them
                    for enode in &eclass.nodes {
                        eprint!(" {:?}", enode);
                    }
            
                    // Newline after each e-class
                    eprintln!();
                }
    // Create a map to hold the connections for each eclass
    let mut connections: HashMap<Id, HashSet<Id>> = HashMap::new();

    for class in egraph.classes() {
        let class_id = class.id;    

        // Initialize the set of connections if not already present
        let mut class_connections = HashSet::new();
        // eprintln!("The size of this eclass with id {:?} is : {:?}", class_id, class.len());

        for (_node_index, node) in class.iter().enumerate() {
            // Print the content of each enode
            // eprintln!("  Enode {}: {:?}", node_index + 1, node);
            for child in node.children() {
                // eprintln!("    Child: {:?}", child);
                // Add child to the list of connections
                class_connections.insert(*child);
            }
        }

        connections.insert(class_id, class_connections);
    }

    // Print the graph in the terminal
    for (class_id, class_connections) in &connections {
        let connections_str: String = class_connections
            .iter()
            .map(|id| id.to_string())
            .collect::<Vec<String>>()
            .join(", ");

        eprintln!("Class {} linked to {}", class_id, connections_str);
    }
}
/*************************************/
pub fn vectorization_rules(vector_width: usize) -> Vec<Rewrite<VecLang, ConstantFold>> {
    let mut rules: Vec<Rewrite<VecLang, ConstantFold>> = vec![];

    let mut searcher_add = Vec::new();
    let mut searcher_mul = Vec::new();
    let mut searcher_sub = Vec::new();
    let mut searcher_neg = Vec::new();

    let mut applier_1 = Vec::new();
    let mut applier_2 = Vec::new();

    for i in 0..vector_width {
        searcher_add.push(format!("( + ?a{} ?b{}) ", i, i));
        searcher_mul.push(format!("( * ?a{} ?b{}) ", i, i));
        searcher_sub.push(format!("( - ?a{} ?b{}) ", i, i));
        searcher_neg.push(format!("( - ?a{}) ", i));

        applier_1.push(format!("?a{} ", i));
        applier_2.push(format!("?b{} ", i));
    }

    let lhs_add: Pattern<VecLang> = format!("(Vec {})", searcher_add.concat()).parse().unwrap();
    let lhs_mul: Pattern<VecLang> = format!("(Vec {})", searcher_mul.concat()).parse().unwrap();
    let lhs_sub: Pattern<VecLang> = format!("(Vec {})", searcher_sub.concat()).parse().unwrap();
    let lhs_neg: Pattern<VecLang> = format!("(Vec {})", searcher_neg.concat()).parse().unwrap();

    // Parse the right-hand side patterns
    let rhs_add: Pattern<VecLang> = format!(
        "(VecAdd (Vec {}) (Vec {}))",
        applier_1.concat(),
        applier_2.concat()
    )
    .parse()
    .unwrap();
    eprintln!("{} => {}", lhs_add, rhs_add);
    let rhs_mul: Pattern<VecLang> = format!(
        "(VecMul (Vec {}) (Vec {}))",
        applier_1.concat(),
        applier_2.concat()
    )
    .parse()
    .unwrap();

    let rhs_sub: Pattern<VecLang> = format!(
        "(VecMinus (Vec {}) (Vec {}))",
        applier_1.concat(),
        applier_2.concat()
    )
    .parse()
    .unwrap();

    let rhs_neg: Pattern<VecLang> = format!("(VecNeg (Vec {}) )", applier_1.concat(),)
        .parse()
        .unwrap();

    // Push the rewrite rules into the rules vector

    rules.push(rw!(format!("add-vectorize" ); { lhs_add.clone() } => { rhs_add.clone() }));
    rules.push(rw!(format!("mul-vectorize"); { lhs_mul.clone() } => { rhs_mul.clone() }));
    rules.push(rw!(format!("sub-vectorize"); { lhs_sub.clone() } => { rhs_sub.clone() }));
    rules.push(rw!(format!("neg-vectorize"); { lhs_neg.clone() } => { rhs_neg.clone() }));
    rules
}
/******************************************/
pub fn is_not_vector_of_scalar_operations(
    vars: &'static str, // Make vars static
) -> impl Fn(&mut Egraph, Id, &Subst) -> bool + 'static {
    let vars = &vars[5..vars.len() - 2];
    let vars_vector = vars.split(" ").collect::<Vec<&str>>();
    move |egraph, _, subst| {
        let mut no_scalar_operations = true;
        for var in &vars_vector {
            let var = var.parse().unwrap();
            no_scalar_operations = no_scalar_operations
                && egraph[subst[var]].nodes.iter().any(|n| match n {
                    VecLang::Num(..) | VecLang::Symbol(..) => true,
                    _ => false,
                });
            if !no_scalar_operations {
                break;
            }
        }
        return no_scalar_operations;
    }
}
/********************************************/
pub fn split_vectors(vector_width: usize) -> Vec<Rewrite<VecLang, ConstantFold>> {
    let mut rules: Vec<Rewrite<VecLang, ConstantFold>> = vec![];

    // Store vector width in a constant

    /************************** Zakaria implementaion *********************************/

    let lhs = format!(
        "(Vec {})",
        (0..vector_width)
            .map(|i| format!("?a{} ", i))
            .collect::<String>()
    );

    let searcher: Pattern<VecLang> = lhs.parse().unwrap();

    for i in 0..vector_width {
        let vector1_add = format!(
            "(Vec {})",
            (0..vector_width)
                .map(|j| if i == j {
                    "0 ".to_string()
                } else {
                    format!("?a{} ", j)
                })
                .collect::<String>()
        );
        let vector1_mul = format!(
            "(Vec {})",
            (0..vector_width)
                .map(|j| if i == j {
                    "1 ".to_string()
                } else {
                    format!("?a{} ", j)
                })
                .collect::<String>()
        );

        let vector2_add = format!(
            "(Vec {})",
            (0..vector_width)
                .map(|j| if i == j {
                    format!("?a{} ", j)
                } else {
                    "0 ".to_string()
                })
                .collect::<String>()
        );
        let vector2_mul = format!(
            "(Vec {})",
            (0..vector_width)
                .map(|j| if i == j {
                    format!("?a{} ", j)
                } else {
                    "1 ".to_string()
                })
                .collect::<String>()
        );

        let rhs_add = format!("(VecAdd {} {})", vector1_add, vector2_add);
        let rhs_mul = format!("(VecMul {} {})", vector1_mul, vector2_mul);
        let applier_add: Pattern<VecLang> = rhs_add.parse().unwrap();
        let applier_mul: Pattern<VecLang> = rhs_mul.parse().unwrap();

        rules.push(rw!(format!("exp-split-add-{}", i); {  searcher.clone()} => {  applier_add}));
        rules.push(rw!(format!("exp-split-mul-{}", i); {  searcher.clone()} => {  applier_mul}))
    }
    rules
}
/*******************************************/
pub fn commutativity_rules(vector_width: usize) -> Vec<Rewrite<VecLang, ConstantFold>> {
    let mut rules: Vec<Rewrite<VecLang, ConstantFold>> = vec![];

    for i in (0..vector_width).step_by(2) {
        // Create the lhs and rhs expressions directly as strings
        let lhs = format!("(+ (* a{} b{}) c{})", i, i, i);
        let rhs = format!("(+ c{} (* a{} b{}))", i, i, i);

        // Parse the expressions into patterns
        let lhs_pattern: Pattern<VecLang> = lhs.parse().unwrap();
        let rhs_pattern: Pattern<VecLang> = rhs.parse().unwrap();

        // Add the rewrite rule using a literal string for the rule name
        rules.push(rw!(format!("exp-assoc-{}", i); lhs_pattern => rhs_pattern));
    }

    rules
}
/******************************************************************************************/
/********************************Conditions ***********************************************/
/******************************************************************************************/
fn is_vec(var1: &'static str,var2: &'static str,var3: &'static str,var4: &'static str) -> impl Fn(&mut EGraph<VecLang, ConstantFold>, Id, &Subst) -> bool {
    let var1_str = var1.parse().unwrap();
    let var2_str = var2.parse().unwrap();
    let var3_str = var3.parse().unwrap();
    let var4_str = var4.parse().unwrap();
    move |egraph : &mut EGraph<VecLang, ConstantFold>, _, subst| {
        let nodes1 = &egraph[subst[var1_str]].nodes ;
        let nodes2 = &egraph[subst[var2_str]].nodes ;
        let nodes3 = &egraph[subst[var3_str]].nodes ;
        let nodes4 = &egraph[subst[var4_str]].nodes ;
        let is_vector1 = nodes1.iter().all(|n| matches!(n, VecLang::Vec(_) | VecLang::VecAdd(_) | VecLang::VecMul(_) | VecLang::VecMinus(_) | VecLang::VecNeg(_)));
        let is_vector2 = nodes2.iter().all(|n| matches!(n, VecLang::Vec(_) | VecLang::VecAdd(_) | VecLang::VecMul(_) | VecLang::VecMinus(_) | VecLang::VecNeg(_)));
        let is_vector3 = nodes3.iter().all(|n| matches!(n, VecLang::Vec(_) | VecLang::VecAdd(_) | VecLang::VecMul(_) | VecLang::VecMinus(_) | VecLang::VecNeg(_)));
        let is_vector4 = nodes4.iter().all(|n| matches!(n, VecLang::Vec(_) | VecLang::VecAdd(_) | VecLang::VecMul(_) | VecLang::VecMinus(_) | VecLang::VecNeg(_)));
        is_vector1&&is_vector2&&is_vector3&&is_vector4
    }
}
/****************************************/
fn is_vec_mul(var1: &'static str,var2: &'static str,var3: &'static str,var4: &'static str,var5: &'static str,var6: &'static str,var7: &'static str,var8: &'static str) -> impl Fn(&mut EGraph<VecLang, ConstantFold>, Id, &Subst) -> bool {
    let var1_str = var1.parse().unwrap();
    let var2_str = var2.parse().unwrap();
    let var3_str = var3.parse().unwrap();
    let var4_str = var4.parse().unwrap();
    let var5_str = var5.parse().unwrap();
    let var6_str = var6.parse().unwrap();
    let var7_str = var7.parse().unwrap();
    let var8_str = var8.parse().unwrap();
    move |egraph : &mut EGraph<VecLang, ConstantFold>, _, subst| {
        let nodes1 = &egraph[subst[var1_str]].nodes ;
        let nodes2 = &egraph[subst[var2_str]].nodes ;
        let nodes3 = &egraph[subst[var3_str]].nodes ;
        let nodes4 = &egraph[subst[var4_str]].nodes ;
        let nodes5 = &egraph[subst[var5_str]].nodes ;
        let nodes6 = &egraph[subst[var6_str]].nodes ;
        let nodes7 = &egraph[subst[var7_str]].nodes ;
        let nodes8 = &egraph[subst[var8_str]].nodes ;
        let is_vector1 = nodes1.iter().all(|n| matches!(n, VecLang::Vec(_) | VecLang::VecAdd(_) | VecLang::VecMul(_) | VecLang::VecMinus(_) | VecLang::VecNeg(_)));
        let is_vector2 = nodes2.iter().all(|n| matches!(n, VecLang::Vec(_) | VecLang::VecAdd(_) | VecLang::VecMul(_) | VecLang::VecMinus(_) | VecLang::VecNeg(_)));
        let is_vector3 = nodes3.iter().all(|n| matches!(n, VecLang::Vec(_) | VecLang::VecAdd(_) | VecLang::VecMul(_) | VecLang::VecMinus(_) | VecLang::VecNeg(_)));
        let is_vector4 = nodes4.iter().all(|n| matches!(n, VecLang::Vec(_) | VecLang::VecAdd(_) | VecLang::VecMul(_) | VecLang::VecMinus(_) | VecLang::VecNeg(_)));
        let is_vector5 = nodes5.iter().all(|n| matches!(n, VecLang::Vec(_) | VecLang::VecAdd(_) | VecLang::VecMul(_) | VecLang::VecMinus(_) | VecLang::VecNeg(_)));
        let is_vector6 = nodes6.iter().all(|n| matches!(n, VecLang::Vec(_) | VecLang::VecAdd(_) | VecLang::VecMul(_) | VecLang::VecMinus(_) | VecLang::VecNeg(_)));
        let is_vector7 = nodes7.iter().all(|n| matches!(n, VecLang::Vec(_) | VecLang::VecAdd(_) | VecLang::VecMul(_) | VecLang::VecMinus(_) | VecLang::VecNeg(_)));
        let is_vector8 = nodes8.iter().all(|n| matches!(n, VecLang::Vec(_) | VecLang::VecAdd(_) | VecLang::VecMul(_) | VecLang::VecMinus(_) | VecLang::VecNeg(_)));

        is_vector1&&is_vector2&&is_vector3&&is_vector4&&is_vector5&&is_vector6&&is_vector7&&is_vector8
    }
}
/***************************************/
fn has_vec_parent(egraph: &EGraph<VecLang, ConstantFold>, eclass_id: Id) -> bool {
    // Use iter() to explicitly iterate over classes
    for eclass in egraph.classes() {
        // eclass is a reference to EClass<VecLang, Option<i32>>, so we don't need to destructure
        for enode in &eclass.nodes {
            if let VecLang::Vec(children) = enode {
                // If the Vec node contains the eclass_id as a child, return true
                if children.contains(&eclass_id) {
                    return true;
                }
            }
        }
    }
    // If no Vec parent found, return false
    false
}
/***************************************/
fn is_leaf(var1: &'static str,var2: &'static str) -> impl Fn(&mut EGraph<VecLang, ConstantFold>, Id, &Subst) -> bool {
    let var1_str = var1.parse().unwrap();
    let var2_str = var2.parse().unwrap();
    move |egraph : &mut EGraph<VecLang, ConstantFold>, _, subst| {
        let nodes1 = &egraph[subst[var1_str]].nodes ;
        let nodes2 = &egraph[subst[var2_str]].nodes ;
        let is_leaf1 = nodes1.iter().all(|enode| enode.children().is_empty());    
        let is_leaf2 = nodes2.iter().all(|enode| enode.children().is_empty());
        let inf = (nodes1.len()==1) && (nodes2.len() == 1) ; 
        //let inf = (nodes1.len()==1) && (nodes2.len() == 1) && !has_vec_parent(egraph, subst[var1_str]) && !has_vec_parent(egraph, subst[var2_str]); 
        // Return true if both e-classes are leaves
        is_leaf1&&is_leaf2&&inf
    }
}
/***************************************/
pub fn cond_check_not_all_values_eq1(vector_width: usize)-> impl Fn(&mut EGraph<VecLang, ConstantFold>, Id, &Subst) -> bool {
    move |egraph : &mut EGraph<VecLang, ConstantFold>, _, subst| {
        (0..vector_width).any(|i| {
            //let var2_str = var2.parse().unwrap();
            let bi = subst[format!("?b{}", i).as_str().parse().unwrap()];
            let ai = subst[format!("?a{}", i).as_str().parse().unwrap()];
            (!&egraph[bi].nodes.iter().any(|node| matches!(node, VecLang::Num(1))))&&(!&egraph[ai].nodes.iter().any(|node| matches!(node, VecLang::Num(1))))
        })
    }
}
/***************************************/
pub fn cond_check_not_all_values_eq0(vector_width: usize)-> impl Fn(&mut EGraph<VecLang, ConstantFold>, Id, &Subst) -> bool {


    move |egraph : &mut EGraph<VecLang, ConstantFold>, _, subst| {
        (0..vector_width).any(|i| {
            //let var2_str = var2.parse().unwrap();
            let bi = subst[format!("?b{}", i).as_str().parse().unwrap()];
            let ai = subst[format!("?a{}", i).as_str().parse().unwrap()];
            (!&egraph[bi].nodes.iter().any(|node| matches!(node, VecLang::Num(0))))&&(!&egraph[bi].nodes.iter().any(|node| matches!(node, VecLang::Num(0))))
        })
    }
}
/***************************************/
pub fn cond_check_all_elems_composed(vector_width: usize)-> impl Fn(&mut EGraph<VecLang, ConstantFold>, Id, &Subst) -> bool {
    move |egraph : &mut EGraph<VecLang, ConstantFold>, _, subst| {
        (0..vector_width).all(|i| {
            //let var2_str = var2.parse().unwrap();
            let bi = subst[format!("?b{}", i).as_str().parse().unwrap()];
            let ai = subst[format!("?a{}", i).as_str().parse().unwrap()];
            (egraph[bi].nodes.iter().all(|node| matches!(node, VecLang::Add(_)| VecLang::Mul(_) | VecLang::Minus(_) )))&&(egraph[ai].nodes.iter().all(|node| matches!(node, VecLang::Add(_) | VecLang::Mul(_) | VecLang::Minus(_) )))
        })
    } 
} 
/***************************************/
pub fn cond_check_any_elems_composed(vector_width: usize)-> impl Fn(&mut EGraph<VecLang, ConstantFold>, Id, &Subst) -> bool {
    move |egraph: &mut EGraph<VecLang, ConstantFold>, _, subst| {
        let count = (0..vector_width).filter(|&i| {
            let bi = subst[format!("?b{}", i).as_str().parse().unwrap()];
            let ai = subst[format!("?a{}", i).as_str().parse().unwrap()];
            
            egraph[bi].nodes.iter().all(|node| matches!(node, VecLang::Add(_) | VecLang::Mul(_) | VecLang::Minus(_))) &&
            egraph[ai].nodes.iter().all(|node| matches!(node, VecLang::Add(_) | VecLang::Mul(_) | VecLang::Minus(_)))
        }).count();
        let count1 = (0..vector_width).filter(|&i| {
            let bi = subst[format!("?b{}", i).as_str().parse().unwrap()];
            let ai = subst[format!("?a{}", i).as_str().parse().unwrap()];
            
            egraph[bi].nodes.iter().all(|node| matches!(node, VecLang::Mul(_))) || egraph[ai].nodes.iter().all(|node| matches!(node, VecLang::Mul(_)))
        }).count();
        count * 2 >= vector_width || count1 >= 1 // Check if at least half of the elements satisfy the condition
    }
}
/*******************************operation rules*************************************************/
/***********************************************************************************************/
pub fn addition_rules(vector_width: usize, expression_depth: usize) -> Vec<Rewrite<VecLang, ConstantFold>>{
    let base: usize = 2;
    let mut max_vector_size : usize = base.pow(expression_depth as u32 - 1) * vector_width; 
    max_vector_size = min(max_vector_size,4096);
    let mut rules: Vec<Rewrite<VecLang, ConstantFold>> = vec![
        rw!("add-0-0+0"; "0" => "(+ 0 0)"),
        rw!("add-a-a+0"; "?a" => 
        "(+ ?a 0)"
        if is_leaf("?a","?a")
        ),
        rw!("add-a*b-0+a*b"; "(* ?a ?b)" => 
        "(+ 0 (* ?a ?b))"
        ),
        rw!("add-a-b-0+a-b"; "(- ?a ?b)" => 
        "(+ 0 (- ?a ?b))"
        ),
        rw!("add--a-0+-a"; "(- ?a)" => 
        "(+ 0 (- ?a))" 
        ),
        rw!("neg-0-0+0"; "0" => "(- 0)"),
    ];
    let mut initial_vector_size : usize = 1 ;
    while initial_vector_size <= max_vector_size{
        let mut searcher_add = Vec::new();
        let mut applier_1 = Vec::new();
        let mut applier_2 = Vec::new();
        for i in 0..initial_vector_size {
            searcher_add.push(format!("( + ?a{} ?b{}) ", i, i));
            applier_1.push(format!("?a{} ", i));
            applier_2.push(format!("?b{} ", i));
        }
        let lhs_add: Pattern<VecLang> = format!("(Vec {})", searcher_add.concat()).parse().unwrap();
        // Parse the right-hand side patterns
        let rhs_add: Pattern<VecLang> = format!(
            "(VecAdd (Vec {}) (Vec {}))",
            applier_1.concat(),
            applier_2.concat()
        )
        .parse()
        .unwrap();
        // Push the rewrite rules into the rules vector
        rules.push(rw!(format!("add-vectorize-{}",initial_vector_size); { lhs_add.clone() } => { rhs_add.clone() } 
        if cond_check_not_all_values_eq0(initial_vector_size)
        ));
        initial_vector_size=initial_vector_size*2
    }
    /***************************************************************/
    max_vector_size = base.pow(expression_depth as u32 - 2) * vector_width ;
    max_vector_size = min(max_vector_size,4096);
    initial_vector_size = 1 ;
    while initial_vector_size <= max_vector_size {
        let mut searcher_add_red = Vec::with_capacity(initial_vector_size);
        let mut applier_1 = Vec::with_capacity(initial_vector_size*2);
        applier_1.resize(initial_vector_size*2,String::from(""));
        for i in 0..initial_vector_size {
            searcher_add_red.push(format!("( + ?a{} ?b{}) ", i, i));
            applier_1[i]=format!("?a{} ", i);
            applier_1[i+initial_vector_size]=format!("?b{} ", i);
        }
        let lhs_add: Pattern<VecLang> = format!("(Vec {})", searcher_add_red.concat()).parse().unwrap();
        // Parse the right-hand side patterns
        let rhs_add: Pattern<VecLang> = format!(
            "(VecAddRot (Vec {}) {})",
            applier_1.concat(),
            initial_vector_size
        )
        .parse()
        .unwrap();
        rules.push(rw!(format!("rot-add-vectorize-{}",initial_vector_size); { lhs_add.clone() } => { rhs_add.clone() } 
        if cond_check_all_elems_composed(initial_vector_size)
        ));
        initial_vector_size=initial_vector_size*2;
    }
    /*************************************************/
    initial_vector_size = 1 ;
    while initial_vector_size <= max_vector_size{
        let mut searcher_neg = Vec::new();
        let mut applier_1 = Vec::new();
        for i in 0..initial_vector_size {
            searcher_neg.push(format!("( - ?b{}) ", i));
            applier_1.push(format!("?b{} ", i));
        }
        let lhs_neg: Pattern<VecLang> = format!("(Vec {})", searcher_neg.concat()).parse().unwrap();
        let rhs_neg: Pattern<VecLang> = format!("(VecNeg (Vec {}) )", applier_1.concat(),)
        .parse()
        .unwrap();
        // Push the rewrite rules into the rules vector
        rules.push(rw!(format!("neg-vectorize-{}",initial_vector_size); { lhs_neg.clone() } => { rhs_neg.clone() }));
        initial_vector_size=initial_vector_size*2 ;
    }
    /*************************************************************/
    /*************************************************************
    max_vector_size = base.pow(expression_depth as u32 - 1) * vector_width; 
    max_vector_size = min(max_vector_size,4096);
    initial_vector_size = 1 ;
    while initial_vector_size <= max_vector_size{
        let mut searcher_mul = Vec::new();
        let mut applier_1 = Vec::new();
        let mut applier_2 = Vec::new();
        for i in 0..initial_vector_size {
            searcher_mul.push(format!("( * ?a{} ?b{}) ", i, i));
            applier_1.push(format!("?a{} ", i));
            applier_2.push(format!("?b{} ", i));
        }
        let lhs_mul: Pattern<VecLang> = format!("(Vec {})", searcher_mul.concat()).parse().unwrap();
        // Parse the right-hand side patterns
        let rhs_mul: Pattern<VecLang> = format!(
            "(VecMul (Vec {}) (Vec {}))",
            applier_1.concat(),
            applier_2.concat()
        )
        .parse()
        .unwrap();
        // Push the rewrite rules into the rules vector
        rules.push(rw!(format!("mul-vectorize-{}",initial_vector_size); { lhs_mul.clone() } => { rhs_mul.clone() } 
        if cond_check_not_all_values_eq1(initial_vector_size)
        ));
        initial_vector_size=initial_vector_size*2;
    }
    /**********************************************/
    max_vector_size = base.pow(expression_depth as u32 - 2) * vector_width;
    max_vector_size = min(max_vector_size,4096); 
    initial_vector_size = 1 ;
    while initial_vector_size <= max_vector_size {
        let mut searcher_mul_red = Vec::with_capacity(initial_vector_size);
        let mut applier_1 = Vec::with_capacity(initial_vector_size*2);
        applier_1.resize(initial_vector_size*2,String::from(""));
        for i in 0..initial_vector_size {
            searcher_mul_red.push(format!("( * ?a{} ?b{}) ", i, i));
            applier_1[i]=format!("?a{} ", i);
            applier_1[i+initial_vector_size]=format!("?b{} ", i);
        }
        let lhs_add: Pattern<VecLang> = format!("(Vec {})", searcher_mul_red.concat()).parse().unwrap();
        // Parse the right-hand side patterns
        let rhs_add: Pattern<VecLang> = format!(
            "(VecMulRot (Vec {}) {})",
            applier_1.concat(),
            initial_vector_size
        )
        .parse()
        .unwrap();
        rules.push(rw!(format!("rot-mul-vectorize-{}",initial_vector_size); { lhs_add.clone() } => { rhs_add.clone() } 
        if cond_check_all_elems_composed(initial_vector_size)
        ));
        initial_vector_size=initial_vector_size*2;
    }
    *************************************************/
    rules
}
/*************************************************************/
pub fn minus_rules(vector_width: usize, expression_depth: usize) -> Vec<Rewrite<VecLang, ConstantFold>>{
    let base: usize = 2;
    let mut max_vector_size : usize = base.pow(expression_depth as u32 - 1) * vector_width; 
    max_vector_size = min(max_vector_size,4096);
    let mut rules: Vec<Rewrite<VecLang, ConstantFold>> = vec![
        rw!("sub-0-0-0"; "0" => "(- 0 0)"),
        rw!("sub-a-a-0"; "?a" => 
        "(- ?a 0)"
        if is_leaf("?a","?a")
        ),        
        /*rw!("part-fold-assoc-add-sub-"; "(- (+ ?b ?c) ?a)" => 
        "(- a? (+ ?b ?c))"
        if is_leaf("?a","?a")
        ),
        rw!("part-fold-assoc-add-sub-"; "(- (* ?b ?c) ?a)" =>
        "(- a? (* ?b ?c))"
        if is_leaf("?a","?a")
        ),
        rw!("part-fold-assoc-sub-1-1"; "(- ?a (- ?b ?c))" => 
        "(+ (- ?a ?b) ?c)"
        ),
        rw!("part-fold-assoc-add-sub-"; "(+ (- ?a ?b) ?c)" => "(- (+ ?a ?c) ?b)"),
        rw!("part-fold-assoc-add-sub-"; "(- (- ?a ?b) ?c)" => "(- ?a (- ?c ?b))"),
        rw!("permute-sub"; "(- ?a ?b)" => "(- ?b ?a)"
        if is_not_leaf("?a","?b")
        ),*/
    ];
    let mut initial_vector_size : usize = 1 ;
    while initial_vector_size <= max_vector_size{
        let mut searcher_sub = Vec::new();
        let mut applier_1 = Vec::new();
        let mut applier_2 = Vec::new();
        for i in 0..initial_vector_size {
            searcher_sub.push(format!("( - ?a{} ?b{}) ", i, i));
            applier_1.push(format!("?a{} ", i));
            applier_2.push(format!("?b{} ", i));
        }
        let lhs_sub: Pattern<VecLang> = format!("(Vec {})", searcher_sub.concat()).parse().unwrap();
        // Parse the right-hand side patterns
        let rhs_sub: Pattern<VecLang> = format!(
            "(VecMinus (Vec {}) (Vec {}))",
            applier_1.concat(),
            applier_2.concat()
        )
        .parse()
        .unwrap();
        // Push the rewrite rules into the rules vector
        rules.push(rw!(format!("sub-vectorize-{}",initial_vector_size); { lhs_sub.clone() } => {rhs_sub.clone()} 
        if cond_check_not_all_values_eq0(initial_vector_size)
        ));
        initial_vector_size=initial_vector_size*2
    }
    /*****************************************************/
    max_vector_size = base.pow(expression_depth as u32 - 2) * vector_width ;
    max_vector_size = min(max_vector_size,4096);
    initial_vector_size = 1 ;
    while initial_vector_size <= max_vector_size {
        let mut searcher_sub_red = Vec::with_capacity(initial_vector_size);
        let mut applier_1 = Vec::with_capacity(initial_vector_size*2);
        applier_1.resize(initial_vector_size*2,String::from(""));
        for i in 0..initial_vector_size {
            searcher_sub_red.push(format!("( - ?a{} ?b{}) ", i, i));
            applier_1[i]=format!("?a{} ", i);
            applier_1[i+initial_vector_size]=format!("?b{} ", i);
        }
        let lhs_sub: Pattern<VecLang> = format!("(Vec {})", searcher_sub_red.concat()).parse().unwrap();
        // Parse the right-hand side patterns
        let rhs_sub: Pattern<VecLang> = format!(
            "(VecMinusRot (Vec {}) {})",
            applier_1.concat(),
            initial_vector_size
        )
        .parse()
        .unwrap();    
        rules.push(rw!(format!("rot-min-vectorize-{}",initial_vector_size); { lhs_sub.clone() } => { rhs_sub.clone() } 
        if cond_check_all_elems_composed(initial_vector_size)
        ));
        initial_vector_size=initial_vector_size*2;
    }
    /***********************************************/
    rules
} 
/*************************************************************/
pub fn multiplication_rules(vector_width: usize, expression_depth: usize) -> Vec<Rewrite<VecLang, ConstantFold>>{
    let base: usize = 2;
    let mut max_vector_size : usize = base.pow(expression_depth as u32 - 1) * vector_width; 
    max_vector_size = min(max_vector_size,4096);
    let mut rules: Vec<Rewrite<VecLang, ConstantFold>> = vec![
        rw!("mul-0-0*0"; "0" => "(* 0 0)"),
        rw!("mul-a-a*1"; "?a" => 
        "(* ?a 1)"
        if is_leaf("?a","?a")
        ),
        rw!("mul-a+b-1-a+b"; "(+ ?a ?b)" => 
        "(* 1 (+ ?a ?b))"
        ),
        rw!("mul-a-b-1-a-b"; "(- ?a ?b)" => 
        "(* 1 (- ?a ?b))"
        ),
        rw!("add--a-0+-a"; "(- ?a)" => 
        "(* 1 (- ?a))"
        ),
        
      
    ];
    let mut initial_vector_size : usize = 1 ;
    while initial_vector_size <= max_vector_size{
        let mut searcher_mul = Vec::new();
        let mut applier_1 = Vec::new();
        let mut applier_2 = Vec::new();
        for i in 0..initial_vector_size {
            searcher_mul.push(format!("( * ?a{} ?b{}) ", i, i));
            applier_1.push(format!("?a{} ", i));
            applier_2.push(format!("?b{} ", i));
        }
        let lhs_mul: Pattern<VecLang> = format!("(Vec {})", searcher_mul.concat()).parse().unwrap();
        // Parse the right-hand side patterns
        let rhs_mul: Pattern<VecLang> = format!(
            "(VecMul (Vec {}) (Vec {}))",
            applier_1.concat(),
            applier_2.concat()
        )
        .parse()
        .unwrap();
        // Push the rewrite rules into the rules vector
        rules.push(rw!(format!("mul-vectorize-{}",initial_vector_size); { lhs_mul.clone() } => { rhs_mul.clone() } 
        if cond_check_not_all_values_eq1(initial_vector_size)
        ));
        initial_vector_size=initial_vector_size*2;
    }
    /**********************************************/
    max_vector_size = base.pow(expression_depth as u32 - 2) * vector_width;
    max_vector_size = min(max_vector_size,4096); 
    initial_vector_size = 1 ;
    while initial_vector_size <= max_vector_size {
        let mut searcher_mul_red = Vec::with_capacity(initial_vector_size);
        let mut applier_1 = Vec::with_capacity(initial_vector_size*2);
        applier_1.resize(initial_vector_size*2,String::from(""));
        for i in 0..initial_vector_size {
            searcher_mul_red.push(format!("( * ?a{} ?b{}) ", i, i));
            applier_1[i]=format!("?a{} ", i);
            applier_1[i+initial_vector_size]=format!("?b{} ", i);
        }
        let lhs_add: Pattern<VecLang> = format!("(Vec {})", searcher_mul_red.concat()).parse().unwrap();
        // Parse the right-hand side patterns
        let rhs_add: Pattern<VecLang> = format!(
            "(VecMulRot (Vec {}) {})",
            applier_1.concat(),
            initial_vector_size
        )
        .parse()
        .unwrap();
        rules.push(rw!(format!("rot-mul-vectorize-{}",initial_vector_size); { lhs_add.clone() } => { rhs_add.clone() } 
        if cond_check_all_elems_composed(initial_vector_size)
        ));
        initial_vector_size=initial_vector_size*2;
    }
    /********************************************/
    rules
}
/*************************************************************/
pub fn neg_rules(vector_width : usize, expression_depth: usize) ->  Vec<Rewrite<VecLang, ConstantFold>>{
    let base: usize = 2;
    let mut max_vector_size : usize = base.pow(expression_depth as u32 - 1) * vector_width; 
    max_vector_size = min(max_vector_size,4096);

    let mut rules: Vec<Rewrite<VecLang, ConstantFold>> = vec![
        rw!("neg-0-0+0"; "0" => "(- 0)"),
    ];
    let mut initial_vector_size : usize = 1 ;
    while initial_vector_size <= max_vector_size{
        let mut searcher_neg = Vec::new();
        let mut applier_1 = Vec::new();
        for i in 0..initial_vector_size {
            searcher_neg.push(format!("( - ?b{}) ", i));
            applier_1.push(format!("?b{} ", i));
        }
        let lhs_neg: Pattern<VecLang> = format!("(Vec {})", searcher_neg.concat()).parse().unwrap();
        let rhs_neg: Pattern<VecLang> = format!("(VecNeg (Vec {}) )", applier_1.concat(),)
        .parse()
        .unwrap();
        // Push the rewrite rules into the rules vector
        rules.push(rw!(format!("neg-vectorize-{}",initial_vector_size); { lhs_neg.clone() } => { rhs_neg.clone() }));
        initial_vector_size=initial_vector_size*2 ;
    }
    rules
}
/***************************************/
fn is_not_leaf(var1: &'static str,var2: &'static str) -> impl Fn(&mut EGraph<VecLang, ConstantFold>, Id, &Subst) -> bool {
    let var1_str = var1.parse().unwrap();
    let var2_str = var2.parse().unwrap();
    move |egraph : &mut EGraph<VecLang, ConstantFold>, _, subst| {
        let nodes1 = &egraph[subst[var1_str]].nodes ;
        let nodes2 = &egraph[subst[var2_str]].nodes ;
        let is_leaf1 = nodes1.iter().all(|enode| enode.children().is_empty());    
        let is_leaf2 = nodes2.iter().all(|enode| enode.children().is_empty());
        (!&egraph[subst[var1_str]].nodes.iter().any(|node| matches!(node, VecLang::Num(_))))||(!&egraph[subst[var2_str]].nodes.iter().any(|node| matches!(node, VecLang::Num(_))))
    }
}
/*********************************Simplication rules*******************************************/
/**********************************************************************************************/
pub fn vector_assoc_mul_rules(vector_width: usize) -> Vec<Rewrite<VecLang, ConstantFold>> {
    let mut rules: Vec<Rewrite<VecLang, ConstantFold>> = vec![
        /*************************************************
        
        *******************************************************/
        rw!("assoc-balan-mul-1"; 
        "(VecMul ?x (VecMul ?y (VecMul ?z ?t)))" => 
        "(VecMul (VecMul ?x ?y) (VecMul ?z ?t))"
        if is_vec("?x","?y","?z","?t")
        ),
        rw!("assoc-balan-mul-2"; 
        "(VecMul ?x (VecMul (VecMul ?z ?t) ?y))" => 
        "(VecMul (VecMul ?x ?z) (VecMul ?t ?y))"
        if is_vec("?x","?z","?t","?y")
        ),
        rw!("assoc-balan-mul-3"; 
        "(VecMul (VecMul (VecMul ?x ?y) ?z) ?t)" => 
        "(VecMul (VecMul ?x ?y) (VecMul ?z ?t))"
        if is_vec("?x","?z","?t","?y")
        ),
        rw!("assoc-balan-mul-4"; 
        "(VecMul (VecMul ?x (VecMul ?y ?z)) ?t)" => 
        "(VecMul (VecMul ?x ?y) (VecMul ?z ?t))"
        if is_vec("?x","?z","?t","?y")
        ),
        rw!("assoc-balan-mul-5"; 
        "(VecMul ?x (VecMul (VecMul ?y ?z) ?t))" => 
        "(VecMul (VecMul ?x ?y) (VecMul ?z ?t))"
        if is_vec("?x","?z","?t","?y")
        ),
        rw!("assoc-balan-mul-6"; 
        "(VecMul ?x (VecMul (VecMul ?y ?z) ?t))" => 
        "(VecMul (VecMul ?x ?y) (VecMul ?z ?t))"
        if is_vec("?x","?z","?t","?y")
        ),
    ];
    rules
}
/**************************************************/
pub fn vector_assoc_min_rules(vector_width: usize) -> Vec<Rewrite<VecLang, ConstantFold>> {
    let mut rules: Vec<Rewrite<VecLang, ConstantFold>> = vec![
        rw!("assoc-balan-min-1"; 
        "(VecMinus ?x (VecMinus ?y (VecMinus ?z ?t)))" => 
        "(VecMinus (VecMinus ?x ?y) (VecMinus ?z ?t))"
        if is_vec("?x","?y","?z","?t")
        ),
        rw!("assoc-balan-min-2"; 
        "(VecMinus ?x (VecMinus (VecMinus ?z ?t) ?y))" => 
        "(VecMinus (VecMinus ?x ?z) (VecMinus ?t ?y))"
        if is_vec("?x","?z","?t","?y")
        ),
        rw!("assoc-balan-min-3"; 
        "(VecMinus (VecMinus (VecMinus ?x ?y) ?z) ?t)" => 
        "(VecMinus (VecMinus ?x ?y) (VecMinus ?z ?t))"
        if is_vec("?x","?z","?t","?y")
        ),
        rewrite!("assoc-balan-min-4"; 
        "(VecMinus (VecMinus ?x (VecMinus ?y ?z)) ?t)" => 
        "(VecMinus (VecMinus ?x ?y) (VecMinus ?z ?t))"
        if is_vec("?x","?z","?t","?y")
        ),
    ];
    rules
}
/***************************************************/
pub fn vector_assoc_add_rules(vector_width: usize) -> Vec<Rewrite<VecLang, ConstantFold>> {
    let mut rules: Vec<Rewrite<VecLang, ConstantFold>> = vec![
      
        rw!("assoc-balan-add-1"; 
        "(VecAdd ?x (VecAdd ?y (VecAdd ?z ?t)))" => 
        "(VecAdd (VecAdd ?x ?y) (VecAdd ?z ?t))"
        if is_vec("?x","?y","?z","?t")
        ),
        rw!("assoc-balan-add-2"; 
        "(VecAdd ?x (VecAdd (VecAdd ?z ?t) ?y))" => 
        "(VecAdd (VecAdd ?x ?z) (VecAdd ?t ?y))"
        if is_vec("?x","?z","?t","?y")
        ),
        rw!("assoc-balan-add-3"; 
        "(VecAdd (VecAdd (VecAdd ?x ?y) ?z) ?t)" => 
        "(VecAdd (VecAdd ?x ?y) (VecAdd ?z ?t))"
        if is_vec("?x","?z","?t","?y")
        ),
        rw!("assoc-balan-add-4"; 
        "(VecAdd (VecAdd ?x (VecAdd ?y ?z)) ?t)" => 
        "(VecAdd (VecAdd ?x ?y) (VecAdd ?z ?t))"
        if is_vec("?x","?z","?t","?y")
        ),
    ];
    rules 
}
/***************************************************/
pub fn vector_assoc_add_mul_rules(vector_width: usize) -> Vec<Rewrite<VecLang, ConstantFold>> {
    let mut rules: Vec<Rewrite<VecLang, ConstantFold>> = vec![
        rw!("assoc-balan-add-mul-1"; 
        "(VecAdd (VecAdd (VecAdd (VecMul ?c1 ?c2) (VecMul ?d1 ?d2)) (VecMul ?b1 ?b2)) (VecMul ?a1 ?a2))" => 
        "(VecAdd (VecAdd (VecMul ?a1 ?a2) (VecMul ?b1 ?b2)) (VecAdd (VecMul ?c1 ?c2) (VecMul ?d1 ?d2)))"
        if is_vec_mul("?a1","?a2","?b1","?b2","?c1","?c2","?d1","?d2")
        ),
        rw!("assoc-balan-add-mul-2"; 
        "(VecAdd (VecMul ?a1 ?a2) (VecAdd (VecMul ?b1 ?b2) (VecAdd (VecMul ?c1 ?c2) (VecMul ?d1 ?d2))))" => 
        "(VecAdd (VecAdd (VecMul ?a1 ?a2) (VecMul ?b1 ?b2)) (VecAdd (VecMul ?c1 ?c2) (VecMul ?d1 ?d2)))"
        if is_vec_mul("?a1","?a2","?b1","?b2","?c1","?c2","?d1","?d2")
        ),
        rw!("assoc-balan-add-mul-3"; 
        "(VecAdd (VecAdd (VecMul ?a1 ?a2) (VecAdd (VecMul ?b1 ?b2) (VecMul ?c1 ?c2))) (VecMul ?d1 ?d2))" => 
        "(VecAdd (VecAdd (VecMul ?a1 ?a2) (VecMul ?b1 ?b2)) (VecAdd (VecMul ?c1 ?c2) (VecMul ?d1 ?d2)))"
        if is_vec_mul("?a1","?a2","?b1","?b2","?c1","?c2","?d1","?d2")
        ),
        rw!("assoc-balan-add-mul-4"; 
        "(VecAdd (VecAdd (VecMul ?a ?b) ?c) ?d)" => "(VecAdd (VecMul ?a ?b) (VecAdd ?c ?d))"
        if is_vec("?a","?b","?c","?c")
        ),
        rw!("assoc-balan-add-mul-5"; 
        "(VecAdd ?a (VecAdd ?b (VecMul ?c ?d)))" => "(VecAdd (VecAdd ?a ?b) (VecMul ?c ?d))"
        if is_vec("?a","?b","?c","?d")
        ),
        rw!("distribute-mul-over-add-1"; 
        "(VecMul ?a (VecAdd ?b ?c))" => "(VecAdd (VecMul ?a ?b) (VecMul ?a ?c))"
        if is_vec("?a","?b","?c","?c")
        ),

       
    ];
    rules
}
/***************************************************/
pub fn vector_assoc_add_min_rules(vector_width: usize) -> Vec<Rewrite<VecLang, ConstantFold>> {
    let mut rules: Vec<Rewrite<VecLang, ConstantFold>> = vec![
        /********************************************
       
        *********************************************/
        rw!("assoc-balan-add-min-1"; 
        "(VecAdd (VecAdd (VecAdd (VecMinus ?c1 ?c2) (VecMinus ?d1 ?d2)) (VecMinus ?b1 ?b2)) (VecMinus ?a1 ?a2))" => 
        "(VecAdd (VecAdd (VecMinus ?a1 ?a2) (VecMinus ?b1 ?b2)) (VecAdd (VecMinus ?c1 ?c2) (VecMinus ?d1 ?d2)))"
        if is_vec_mul("?a1","?a2","?b1","?b2","?c1","?c2","?d1","?d2")
        ),
        rw!("assoc-balan-add-min-2"; 
        "(VecAdd (VecMinus ?a1 ?a2) (VecAdd (VecMinus ?b1 ?b2) (VecAdd (VecMinus ?c1 ?c2) (VecMinus ?d1 ?d2))))" => 
        "(VecAdd (VecAdd (VecMinus ?a1 ?a2) (VecMinus ?b1 ?b2)) (VecAdd (VecMinus ?c1 ?c2) (VecMinus ?d1 ?d2)))"
        if is_vec_mul("?a1","?a2","?b1","?b2","?c1","?c2","?d1","?d2")
        ),
        rw!("assoc-balan-add-min-3"; 
        "(VecAdd (VecAdd (VecMinus ?a1 ?a2) (VecAdd (VecMinus ?b1 ?b2) (VecMinus ?c1 ?c2))) (VecMinus ?d1 ?d2))" => 
        "(VecAdd (VecAdd (VecMinus ?a1 ?a2) (VecMinus ?b1 ?b2)) (VecAdd (VecMinus ?c1 ?c2) (VecMinus ?d1 ?d2)))"
        if is_vec_mul("?a1","?a2","?b1","?b2","?c1","?c2","?d1","?d2")
        ),
    ];
    rules
}
/****************************************************/
pub fn vector_assoc_min_mul_rules(vector_width: usize) -> Vec<Rewrite<VecLang, ConstantFold>> {
    let mut rules: Vec<Rewrite<VecLang, ConstantFold>> = vec![
        rewrite!("assoc-balan-min-mul-1"; 
        "(VecMinus (VecMinus (VecMinus (VecMul ?c1 ?c2) (VecMul ?d1 ?d2)) (VecMul ?b1 ?b2)) (VecMul ?a1 ?a2))" => 
        "(VecMinus (VecMinus (VecMul ?a1 ?a2) (VecMul ?b1 ?b2)) (VecAdd (VecMul ?c1 ?c2) (VecMul ?d1 ?d2)))"
        if is_vec_mul("?a1","?a2","?b1","?b2","?c1","?c2","?d1","?d2")
        ),
        rewrite!("assoc-balan-min-mul-2"; 
        "(VecMinus (VecMul ?a1 ?a2) (VecMinus (VecMul ?b1 ?b2) (VecMinus (VecMul ?c1 ?c2) (VecMul ?d1 ?d2))))" => 
        "(VecMinus (VecMinus (VecMul ?a1 ?a2) (VecMul ?b1 ?b2)) (VecMinus (VecMul ?c1 ?c2) (VecMul ?d1 ?d2)))"
        if is_vec_mul("?a1","?a2","?b1","?b2","?c1","?c2","?d1","?d2")
        ),
        rewrite!("assoc-balan-min-mul-3"; 
        "(VecMinus (VecMinus (VecMul ?a1 ?a2) (VecMinus (VecMul ?b1 ?b2) (VecMul ?c1 ?c2))) (VecMul ?d1 ?d2))" => 
        "(VecMinus (VecMinus (VecMul ?a1 ?a2) (VecMul ?b1 ?b2)) (VecMinus (VecMul ?c1 ?c2) (VecMul ?d1 ?d2)))"
        if is_vec_mul("?a1","?a2","?b1","?b2","?c1","?c2","?d1","?d2")
        ),
        /*rewrite!("distribute-mul-over-min"; 
            "(VecMul ?a (VecMinus ?b ?c))" => "(VecMinus (VecMul ?a ?b) (VecMul ?a ?c))"
            if is_vec("?a","?b","?c","?c")
        ),
        rewrite!("factor-out-mul_min";
            "(VecMinus (VecMul ?a ?b) (VecMul ?a ?c))" => "(VecMul ?a (VecMinus ?b ?c))"
            //if is_vec("?a","?b","?c","?c")
        ),*/
    ];
    rules
}
/*****************************************************/
pub fn assoc_neg_rules(vector_width : usize) -> Vec<Rewrite<VecLang,ConstantFold>>{
    let mut rules: Vec<Rewrite<VecLang, ConstantFold>> = vec![
        rewrite!("simplify-sub-negate"; 
        "(VecMinus ?x (VecNeg ?y))" => 
        "(VecAdd ?x ?y)"
        if is_vec("?x","?x","?y","?y")
        ),
        rewrite!("simplify-sub-negate-1"; 
        "(VecAdd ?x (VecNeg ?y))" => 
        "(VecMinus ?x ?y)"
        if is_vec("?x","?x","?y","?y")
        ),
        rewrite!("simplify-sub-negate-1-2"; 
        "(VecAdd (VecNeg ?y) ?x)" => 
        "(VecMinus ?x ?y)"
        if is_vec("?x","?x","?y","?y")
        ),
        rewrite!("simplify-add-mul-negate-1"; 
        "(VecAdd (VecMul ?x (VecNeg ?y)) ?z)" => 
        "(VecMinus ?z (VecMul ?x ?y))"
        if is_vec("?x","?x","?y","?z")
        ), 
        rewrite!("simplify-add-mul-negate-2"; 
        "(VecAdd (VecMul (VecNeg ?y) ?x) ?z)" => 
        "(VecMinus ?z (VecMul ?x ?y))"
        if is_vec("?x","?x","?y","?z")
        ),
        rewrite!("simplify-add-mul-negate-3"; 
        "(VecAdd ?z (VecMul ?x (VecNeg ?y)))" => 
        "(VecMinus ?z (VecMul ?x ?y))"
        if is_vec("?x","?x","?y","?z")
        ),
        rewrite!("simplify-add-mul-negate-4"; 
        "(VecAdd ?z (VecMul (VecNeg ?y) ?x))" => 
        "(VecMinus ?z (VecMul ?y ?x))"
        if is_vec("?x","?x","?y","?z")
        ),
        rewrite!("simplify-sub-mul-negate-1"; 
        "(VecMinus ?z (VecMul ?x (VecNeg ?y)))" => 
        "(VecAdd ?z (VecMul ?x ?y))"
        if is_vec("?x","?x","?y","?z")
        ),
        rewrite!("simplify-sub-mul-negate-2"; 
        "(VecMinus ?z (VecMul (VecNeg ?y) ?x))" => 
        "(VecAdd ?z (VecMul ?x ?y))"
        if is_vec("?x","?x","?y","?z")
        ),
        rewrite!("simplify-add-negate-2-1"; 
        "(VecAdd ?x (VecMinus (VecNeg ?y) ?z))" => 
        "(VecMinus ?x (VecAdd ?x ?y))"
        if is_vec("?x","?x","?y","?z")
        ),
        rewrite!("simplify-add-negate-2-2"; 
        "(VecAdd (VecMinus ?z (VecNeg ?y)) ?x)" => 
        "(VecMinus ?x (VecAdd ?x ?y))"
        if is_vec("?x","?x","?y","?z")
        ),
    ];
    rules
}
/**********************************************************************************************/
// // ===================================================================


// ===================================================================
// 2. ALL REQUIRED HELPER FUNCTIONS
// ===================================================================

/// Creates a single condition that checks if ALL provided variables are scalars.
fn all_are_scalars(vars: &[String]) -> impl Fn(&mut EGraph<VecLang, ConstantFold>, Id, &Subst) -> bool + Clone {
    let vars_to_check: Vec<Var> = vars.iter().map(|s| s.parse().unwrap()).collect();
    move |egraph, _, subst| {
        for var in &vars_to_check {
            if let Some(eclass_id) = subst.get(*var) {
                let eclass = &egraph[*eclass_id];
                if eclass.nodes.len() != 1 || !matches!(&eclass.nodes[0], VecLang::Symbol(_)) {
                    return false;
                }
            } else { return false; }
        }
        true
    }
}

/// Helper to build a balanced tree pattern string in prefix format.
fn build_pattern_from_vars(vars: &mut VecDeque<String>) -> String {
    if vars.is_empty() { return String::new(); }
    if vars.len() == 1 { return vars.pop_front().unwrap(); }
    let mid = vars.len() / 2;
    let mut left_vars = vars.split_off(mid);
    let mut right_vars = std::mem::take(vars);
    let left = build_pattern_from_vars(&mut right_vars);
    let right = build_pattern_from_vars(&mut left_vars);
    format!("(+ {} {})", left, right)
}

/// Helper to build a right-leaning chain pattern.
fn build_chain_from_vars(vars: &mut VecDeque<String>) -> String {
    if vars.is_empty() { return String::new(); }
    if vars.len() == 1 { return vars.pop_front().unwrap(); }
    let first = vars.pop_front().unwrap();
    let rest = build_chain_from_vars(vars);
    format!("(+ {} {})", first, rest)
}

/// Generates and PRINTS a powerful set of generic and size-specific, guarded rewrite rules.
pub fn generate_associativity_rules(max_size: u32) -> Vec<Rewrite<VecLang, ConstantFold>> {
    let mut all_rules: Vec<Rewrite<VecLang, ConstantFold>> = vec![];

    // --- Phase 1: Generic, Unconditional, Structural Rules ---
    // eprintln!("\n=======================================================");
    // eprintln!("[Generic Rules] Generating rules that apply at all scales...");
    // eprintln!("=======================================================");

    // THE DEFINITIVE FIX: Manually create the forward and reverse rules for each transformation.
    all_rules.push(rw!("rebalance-generic-fwd"; "(+ (+ ?a ?b) ?c)" => "(+ ?a (+ ?b ?c))"));
    all_rules.push(rw!("rebalance-generic-rev"; "(+ ?a (+ ?b ?c))" => "(+ (+ ?a ?b) ?c)"));

    all_rules.push(rw!("interleave-generic-fwd"; "(+ (+ ?a ?b) (+ ?c ?d))" => "(+ (+ ?a ?c) (+ ?b ?d))"));
    all_rules.push(rw!("interleave-generic-rev"; "(+ (+ ?a ?c) (+ ?b ?d))" => "(+ (+ ?a ?b) (+ ?c ?d))"));
    
    // eprintln!("  (+) Generic Rule: rebalance-generic-fwd");
    // eprintln!("  (+) Generic Rule: rebalance-generic-rev");
    // eprintln!("  (+) Generic Rule: interleave-generic-fwd");
    // eprintln!("  (+) Generic Rule: interleave-generic-rev");


    // --- Phase 2: Size-Specific, Guarded Rules for Fine Control at the Leaf Level ---
    let mut n = 4;
    while n <= max_size {
        if n.is_power_of_two() {
            // eprintln!("\n=======================================================");
            // eprintln!("[N = {} scalars] Generating explicit, guarded leaf-level rules...", n);
            // eprintln!("=======================================================");

            let vars: Vec<String> = (0..n).map(|i| format!("?v{}", i)).collect();
            let guard = all_are_scalars(&vars);
            let lhs_pattern: Pattern<VecLang> = build_pattern_from_vars(&mut VecDeque::from(vars.clone())).parse().unwrap();
            
            let mut create_rule = |name: String, rhs: Pattern<VecLang>| {
                let rule = Rewrite::new(
                    name,
                    lhs_pattern.clone(),
                    ConditionalApplier { condition: guard.clone(), applier: rhs }
                ).unwrap();
                // eprintln!("  (+) Guarded Rule (N={}): {:?}", n, rule);
                all_rules.push(rule);
            };

            let mid = n as usize / 2;
            let (l_vars, r_vars) = vars.split_at(mid);
            let rhs_swap = build_pattern_from_vars(&mut VecDeque::from([r_vars, l_vars].concat())).parse().unwrap();
            create_rule(format!("swap-explicit-{}", n), rhs_swap);

            let rhs_chain = build_chain_from_vars(&mut VecDeque::from(vars.clone())).parse().unwrap();
            create_rule(format!("group-right-chain-{}", n), rhs_chain);

            let quarter = n as usize / 4;
            let ll_vars = &vars[0..quarter];
            let lr_vars = &vars[quarter..mid];
            let rl_vars = &vars[mid..mid + quarter];
            let rr_vars = &vars[mid + quarter..];
            let rhs_rev_interleave_str = format!("(+ {} {})",
                build_pattern_from_vars(&mut VecDeque::from([ll_vars, rr_vars].concat())),
                build_pattern_from_vars(&mut VecDeque::from([lr_vars, rl_vars].concat()))
            );
            create_rule(format!("group-rev-interleave-{}", n), rhs_rev_interleave_str.parse().unwrap());
            
            if n >= 8 {
                 let center_chunk = build_pattern_from_vars(&mut VecDeque::from([lr_vars, rl_vars].concat()));
                 let left_chunk = build_pattern_from_vars(&mut VecDeque::from(ll_vars.to_vec()));
                 let right_chunk = build_pattern_from_vars(&mut VecDeque::from(rr_vars.to_vec()));
                 let rhs_mixed_str = format!("(+ (+ {} {}) {})", left_chunk, center_chunk, right_chunk);
                 create_rule(format!("group-mixed-assoc-{}", n), rhs_mixed_str.parse().unwrap());
            }

            if n == 4 {
                 create_rule(
                     "catalan-left-chain-4".to_string(),
                     "(+ (+ (+ ?v0 ?v1) ?v2) ?v3)".parse().unwrap()
                 );
                 create_rule(
                     "catalan-mixed-4".to_string(),
                     "(+ ?v0 (+ (+ ?v1 ?v2) ?v3))".parse().unwrap()
                 );
            }
        }
        n *= 2;
    }
    all_rules
}

/// Generates a very rich set of associativity and zero-manipulation rules.
pub fn generate_advanced_rules(max_size: u32) -> Vec<Rewrite<VecLang, ConstantFold>> {
    let mut all_rules: Vec<Rewrite<VecLang, ConstantFold>> = vec![];

    // --- Phase 1: Foundational Generic & Zero Cleanup Rules ---
    // eprintln!("\n=======================================================");
    // eprintln!("[Generic Rules] Generating foundational rules...");
    // eprintln!("=======================================================");

    // THE DEFINITIVE FIX: Manually create two unidirectional rules for each bidirectional one.
    // This resolves the `mismatched types` error permanently.
    all_rules.push(rw!("add-0-elim-fwd"; "(+ ?a 0)" => "?a"));
    all_rules.push(rw!("add-0-elim-rev"; "?a" => "(+ ?a 0)"));
    eprintln!("  (+) Generic Rule: add-0-elim (fwd/rev)");

    all_rules.push(rw!("rebalance-generic-fwd"; "(+ (+ ?a ?b) ?c)" => "(+ ?a (+ ?b ?c))"));
    all_rules.push(rw!("rebalance-generic-rev"; "(+ ?a (+ ?b ?c))" => "(+ (+ ?a ?b) ?c)"));
    // eprintln!("  (+) Generic Rule: rebalance-generic (fwd/rev)");

    all_rules.push(rw!("interleave-generic-fwd"; "(+ (+ ?a ?b) (+ ?c ?d))" => "(+ (+ ?a ?c) (+ ?b ?d))"));
    all_rules.push(rw!("interleave-generic-rev"; "(+ (+ ?a ?c) (+ ?b ?d))" => "(+ (+ ?a ?b) (+ ?c ?d))"));
    // eprintln!("  (+) Generic Rule: interleave-generic (fwd/rev)");
    
    // --- Phase 2: Guarded "Zero Injection" Rules - A Portfolio of Possibilities ---
    // eprintln!("\n=======================================================");
    // eprintln!("[Zero Injection Rules] Generating a portfolio of structural modifications...");
    // eprintln!("=======================================================");
    
    // Unconditional rule to expand a zero. This allows "empty" branches to be created.
    all_rules.push(rw!("zero-expand"; "0" => "(+ 0 0)"));
    // eprintln!("  (+) Zero Expansion Rule: 0 => (+ 0 0)");

    let mut n_zero_inject = 4;
    while n_zero_inject <= max_size {
        if n_zero_inject.is_power_of_two() {
            let vars: Vec<String> = (0..n_zero_inject).map(|i| format!("?v{}", i)).collect();
            let guard = all_are_scalars(&vars);
            let lhs: Pattern<VecLang> = build_pattern_from_vars(&mut VecDeque::from(vars.clone())).parse().unwrap();
            
            let mut create_zero_rule = |name: String, rhs_str: String| {
                let rhs: Pattern<VecLang> = rhs_str.parse().unwrap();
                let rule = Rewrite::new(name, lhs.clone(), ConditionalApplier { condition: guard.clone(), applier: rhs }).unwrap();
                // eprintln!("  (+) Zero Injection Rule (N={}): {:?}", n_zero_inject, rule);
                all_rules.push(rule);
            };

            let mid = n_zero_inject as usize / 2;
            let quarter = n_zero_inject as usize / 4;
            let (l_vars, r_vars) = vars.split_at(mid);
            let left_chunk = build_pattern_from_vars(&mut VecDeque::from(l_vars.to_vec()));
            let right_chunk = build_pattern_from_vars(&mut VecDeque::from(r_vars.to_vec()));
            
            // Possibility 1: Inject into both children
            create_zero_rule(
                format!("zero-inject-children-{}", n_zero_inject),
                format!("(+ (+ {} (+ 0 0)) (+ {} (+ 0 0)))", left_chunk, right_chunk)
            );

            // Possibility 2 & 3 (for larger sizes): Inject deep into grandchildren
            if n_zero_inject >= 8 {
                let ll_chunk = build_pattern_from_vars(&mut VecDeque::from(vars[0..quarter].to_vec()));
                let lr_chunk = build_pattern_from_vars(&mut VecDeque::from(vars[quarter..mid].to_vec()));
                let rl_chunk = build_pattern_from_vars(&mut VecDeque::from(vars[mid..mid+quarter].to_vec()));
                
                // Inject into the two "inner" grandchildren (lr and rl)
                create_zero_rule(
                    format!("zero-inject-inner-grandchildren-{}", n_zero_inject),
                    format!("(+ (+ {} (+ {} (+ 0 0))) (+ (+ {} (+ 0 0)) {}))", ll_chunk, lr_chunk, rl_chunk, right_chunk)
                );
            }
        }
        n_zero_inject *= 2;
    }
    
    // --- Phase 3: Guarded Associativity Rules for Leaf-level Control ---
    // eprintln!("\n=======================================================");
    // eprintln!("[Guarded Associativity] Generating fine-grained leaf-level rules...");
    // eprintln!("=======================================================");
    let n_guarded = 4;
    let vars: Vec<String> = (0..n_guarded).map(|i| format!("?v{}", i)).collect();
    let guard = all_are_scalars(&vars);
    let lhs: Pattern<VecLang> = build_pattern_from_vars(&mut VecDeque::from(vars.clone())).parse().unwrap();
    let rev_interleave: Pattern<VecLang> = "(+ (+ ?v0 ?v3) (+ ?v1 ?v2))".parse().unwrap();
    let rev_interleave_rule = Rewrite::new(
        "interleave-rev-guarded-4",
        lhs,
        ConditionalApplier { condition: guard, applier: rev_interleave }
    ).unwrap();
    // eprintln!("  (+) Guarded Rule (N=4): {:?}", rev_interleave_rule);
    all_rules.push(rev_interleave_rule);

    all_rules
}