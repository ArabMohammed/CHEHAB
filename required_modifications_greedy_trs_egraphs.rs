
///////////////////////////////////////////////////////////////////////
/////// ===> egraphs/src/rules //////////////////////////////////////
pub struct CostedRewrite<L, N> {
    pub rule: Rewrite<L, N>,
    pub cost: usize,
}
impl<L, N> CostedRewrite<L, N> {
    pub fn new(rule: Rewrite<L, N>, cost: usize) -> Self {
        Self { rule, cost }
    }
}
pub fn greedy_trs_rules(vector_width: usize, expression_depth: usize) -> Vec<Rewrite<VecLang, ConstantFold>>{
    let base: usize = 2;
    let mut max_vector_size : usize = base.pow(expression_depth as u32 - 1) * vector_width; 
    /******************* Addition rules  *****************************************/
    /*****************************************************************************/
    max_vector_size = min(max_vector_size,4096);
    let mut costedRules: Vec<CostedRewrite<VecLang, ConstantFold>> = vec![];
    let mut rules: Vec<Rewrite<VecLang, ConstantFold>> = vec![
        /************Addition ********
        //rw!("add-0-0+0"; "0" => "(+ 0 0)"),
        rw!("add-a-a+0"; "?a" => 
        "(+ ?a 0)"
        //if is_leaf("?a","?a")
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
        /********* Subtraction *******/
        rw!("sub-0-0-0"; "0" => "(- 0 0)"),
        rw!("sub-a-a-0"; "?a" => 
        "(- ?a 0)"
        if is_leaf("?a","?a")
        ),
        rw!("sub--a-0--a"; "(- ?a)" => 
        "(- 0  ?a)"
        ),
        /********* Multiplication ********/
        rw!("mul-0-0*0"; "0" => "(* 0 0)"),
        rw!("mul-a-a*1"; "?a" => 
        "(* ?a 1)"
        if is_leaf("?a","?a")
        ),
        rw!("mul-a+b-1-a+b"; "(+ ?a ?b)" => 
        "(* 1 (+ ?a ?b))"
        ),
        rw!("mul-a-b--1"; "(- ?a ?b)" => 
        "(* 1 (- ?a ?b))"
        ),
        rw!("mul-a--1"; "(- ?a)" => 
        "(* 1 (- ?a))"
        ),
        ********************************/

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
        let rule = rw!(
            format!("add-vectorize-{}", initial_vector_size);
            { lhs_add.clone() } => { rhs_add.clone() }
            if cond_check_not_all_values_eq0(initial_vector_size)
        );
        let reduction = (initial_vector_size * 10000 + 2000)- 4001 ;
        costedRules.push(CostedRewrite::new(rule, reduction));
        /**************************************/
        initial_vector_size=initial_vector_size*2;
    }
    /*************************************************/
    initial_vector_size = 1 ;
    max_vector_size = base.pow(expression_depth as u32 - 2) * vector_width ;
    max_vector_size = min(max_vector_size,4096);
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
        //////////
        let rule = rw!(format!("rot-add-vectorize-{}",initial_vector_size); { lhs_add.clone() } => { rhs_add.clone() } 
        if cond_check_all_elems_composed(initial_vector_size)
        );
        let reduction = (initial_vector_size * 10000 + 2000)- 3051 ;
        costedRules.push(CostedRewrite::new(rule, reduction));
        /*********************************************/
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
        let rule = rw!(format!("neg-vectorize-{}",initial_vector_size); { lhs_neg.clone() } => { rhs_neg.clone() });
        let reduction = (initial_vector_size * 10000 + 2000)- 2001 ;
        costedRules.push(CostedRewrite::new(rule, reduction));
        /*******************************************/
        initial_vector_size=initial_vector_size*2 ;
    }
    /*********************** subtraction rules**************************************/
    /*******************************************************************************/
    initial_vector_size = 1 ;
    max_vector_size = base.pow(expression_depth as u32 - 1) * vector_width; 
    max_vector_size = min(max_vector_size,4096);
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
        let rule = rw!(format!("sub-vectorize-{}",initial_vector_size); { lhs_sub.clone() } => {rhs_sub.clone()} 
        if cond_check_not_all_values_eq0(initial_vector_size)
        );
        let reduction = (initial_vector_size * 10000 + 2000)- 4001 ;
        costedRules.push(CostedRewrite::new(rule, reduction));
        /*********************************************/
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
        /////////////////////////////
        let rule = rw!(format!("rot-min-vectorize-{}",initial_vector_size); { lhs_sub.clone() } => { rhs_sub.clone() } 
        if cond_check_all_elems_composed(initial_vector_size)
        );
        let reduction = (initial_vector_size * 10000 + 2000)- 3051 ;
        costedRules.push(CostedRewrite::new(rule, reduction));
        /**********************************************/
        initial_vector_size=initial_vector_size*2;
    }
    /*********************** multiplication rules ****************************************/
    /************************************************************************************/
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
        let rule = rw!(format!("mul-vectorize-{}",initial_vector_size); { lhs_mul.clone() } => { rhs_mul.clone() } 
        if cond_check_not_all_values_eq1(initial_vector_size)
        );
        let reduction = (initial_vector_size * 10000 + 2000)- 4100 ;
        costedRules.push(CostedRewrite::new(rule, reduction));
        /***********************************************/
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
        /////////////////////////
        let rule = rw!(format!("rot-mul-vectorize-{}",initial_vector_size); { lhs_add.clone() } => { rhs_add.clone() } 
        if cond_check_all_elems_composed(initial_vector_size)
        );
        let reduction = (initial_vector_size * 10000 + 2000)- 4150 ;
        costedRules.push(CostedRewrite::new(rule, reduction));
        /***********************************************/
        initial_vector_size=initial_vector_size*2;
    }
    /**************************************************************************************/
    costedRules.sort_by(|a, b| b.cost.cmp(&a.cost));
    for costedRule in costedRules {
        rules.push(costedRule.rule);
    }
    rules
}

///////////////////////////////////////////////////////////////////////
//////////===>  egraphs/src/runner ////////////////////////////////////////////
fn run_one(&mut self, rules: &[&Rewrite<L, N>]) -> Iteration<IterData> {
        assert!(self.stop_reason.is_none());
        //eprintln!("==> run_one 0");
        info!("\nIteration {}", self.iterations.len());
        
        self.try_start();
        let mut result = self.check_limits();

        let egraph_nodes = self.egraph.total_size();
        let egraph_classes = self.egraph.number_of_classes();

        let hook_time = Instant::now();
        //eprintln!("==> run_one 1");
        let mut hooks = std::mem::take(&mut self.hooks);
        //eprintln!("==> run_one 2");
        result = result.and_then(|_| {
            hooks
                .iter_mut()
                .try_for_each(|hook| hook(self).map_err(StopReason::Other))
        });
        self.hooks = hooks;
        let hook_time = hook_time.elapsed().as_secs_f64();
        //eprintln!("==> run_one 3");
        let egraph_nodes_after_hooks = self.egraph.total_size();
        let egraph_classes_after_hooks = self.egraph.number_of_classes();

        let i = self.iterations.len();
        trace!("EGraph {:?}", self.egraph.dump());

        let start_time = Instant::now();
        //eprintln!("==> run_one 4");
        let mut matches = Vec::new();
        let mut applied = IndexMap::default();
        let max_subs_size : usize = 100 ;
        /*********************************************************************/
        //eprintln!("==> run_one 5");
        result = result.and_then(|_| {
            rules.iter().try_for_each(|rw| {
                let start = Instant::now();
                let mut ms = self.scheduler.search_rewrite(i, &self.egraph, rw);
                matches.push(ms);
                let end = start.elapsed();
                self.check_limits()
            })
        });
        let search_time = start_time.elapsed().as_secs_f64();
        
        /******************* Applying Found matches *********************************/
        let apply_time = Instant::now();
        //eprintln!("==> run_one 6");
        /*result = result.and_then(|_| {
            rules.iter().zip(matches).try_for_each(|(rw, ms)| {
                let start = Instant::now();
                let total_matches: usize = ms.iter().map(|m| m.substs.len()).sum();
                debug!("Applying {} {} times", rw.name, total_matches);
                //eprintln!("Applying Rewrite rule : '{}' total_matches : {}", rw.name,total_matches);
                if total_matches == 0 {
                    debug!("No matches for rule '{}'. Skipping application.", rw.name);
                    return Ok(());
                }
                let actually_matched = self.scheduler.apply_rewrite(i, &mut self.egraph, rw, ms);
                if actually_matched > 0 {
                    if let Some(count) = applied.get_mut(&rw.name) {
                        *count += actually_matched;
                    } else {
                        applied.insert(rw.name.to_owned(), actually_matched);
                    }
                    eprintln!("===> {} was applied : {} times", rw.name, actually_matched);
                }
                let end = start.elapsed();
                //eprintln!("time for applying the rewrte rule {:?} is {:?}", rw.name, end);
                self.check_limits()
            })
        });*/
        result = result.and_then(|_| {
            for (rw, ms) in rules.iter().zip(matches) {
                let total_matches: usize = ms.iter().map(|m| m.substs.len()).sum();
                if total_matches == 0 {
                    continue;
                }
                let actually_matched = self.scheduler.apply_rewrite(i, &mut self.egraph, rw, ms);
                if actually_matched > 0 {
                    applied.insert(rw.name.to_owned(), actually_matched);
                    break; // Greedy: stop after first successful rewrite
                }
            }
            Ok(())
        });
        //eprintln!("==> run_one 7");
        /***************************************************************************/
        let apply_time = apply_time.elapsed().as_secs_f64();
        //eprintln!("Total Apply time: {}", apply_time);
        let rebuild_time = Instant::now();
        //eprintln!("===> Next1 :");
        let n_rebuilds = self.egraph.rebuild();
        let rebuild_time = rebuild_time.elapsed().as_secs_f64();
        //eprintln!("===> Next2 :");
        // eprintln!("Rebuild time: {}", rebuild_time);
        info!(
            "Size: n={}, e={}",
            self.egraph.total_size(),
            self.egraph.number_of_classes()
        );
        //eprintln!("===> Next3 :");
        /************************************************/
        let can_be_saturated = applied.is_empty()
            && self.scheduler.can_stop(i)
            // now make sure the hooks didn't do anything
            && (egraph_nodes == egraph_nodes_after_hooks)
            && (egraph_classes == egraph_classes_after_hooks)
            // now make sure that conditional rules (which might add
            // nodes without applying) didn't do anything
            && (egraph_nodes == self.egraph.total_size())
            && (egraph_classes == self.egraph.number_of_classes());

        if can_be_saturated {
            result = result.and(Err(StopReason::Saturated))
        }
        //eprintln!("===> Next4 :");
        /************************************************/
        Iteration {
            applied,
            egraph_nodes,
            egraph_classes,
            hook_time,
            search_time,
            apply_time,
            rebuild_time,
            n_rebuilds,
            data: IterData::make(self),
            total_time: start_time.elapsed().as_secs_f64(),
            stop_reason: result.err(),
        }
    }
/*******************************************************/
fn apply_rewrite(
    &mut self,
    iteration: usize,
    egraph: &mut EGraph<L, N>,
    rewrite: &Rewrite<L, N>,
    matches: Vec<SearchMatches<L>>,
) -> usize {
    let mut total_applied = 0;
    // Step 1: apply rule once to all initial matches
    let added_ids = rewrite.apply(egraph, &matches); // this calls apply_matches
    //  Rebuild before searching again
    egraph.rebuild();
    total_applied += added_ids.len();
    // Step 2: recursively apply to newly created eclasses
    let mut queue: Vec<Id> = added_ids.clone();
    let max_depth = 5; // optional depth limit to avoid infinite loops
    let mut depth = 0;
    while !queue.is_empty() && depth < max_depth {
        let mut new_ids = vec![];

        for eclass_id in queue.drain(..) {
            let inner_matches = rewrite.searcher.search_eclass(egraph, eclass_id);
            if let Some(matches) = inner_matches {
                // test if replacing matched term in the apply function works.
                let applied_ids = rewrite.apply(egraph, std::slice::from_ref(&matches));
                // Rebuild after each application round
                egraph.rebuild();
                // process applied_ids...
                //let applied_ids = rewrite.apply(egraph, &inner_matches);
                total_applied += applied_ids.len();
                new_ids.extend(applied_ids);
            }
        }
        queue = new_ids;
        depth += 1;
    }
    total_applied
}

////////////////// ===> egg/src/pattern.rs ///////////////////////
fn apply_one(
    &self,
    egraph: &mut EGraph<L, A>,
    eclass: Id,
    subst: &Subst,
    searcher_ast: Option<&PatternAst<L>>,
    rule_name: Symbol,
) -> Vec<Id> {
    let ast = self.ast.as_ref();
    let mut id_buf = vec![0.into(); ast.len()];
    let id = apply_pat(&mut id_buf, ast, egraph, subst);
    let old_eclass_nodes = &egraph[eclass].nodes.clone() ;
    if let Some(ast) = searcher_ast {
        let (from, did_something) =
            egraph.union_instantiations(ast, &self.ast, subst, rule_name);
        if did_something {
            // Destructively clear old e-nodes in `from`
            egraph[from].nodes.clear();
            vec![from]
        } else {
            vec![]
        }
    } else if egraph.union(eclass, id) {
        // loop over all old-eclass e-nodes 
        // remove old-nodes from the eclass ofter it update using union operation 
        /*for old_enode in old_eclass_nodes {
            let mut index : usize = 0;
            for enode in &egraph[eclass].nodes {
                if old_enode == enode {
                    egraph[eclass].nodes.remove(index);
                    break;
                }
                index=index+1;
            }
        }*/
        let class = egraph.find(eclass);
        // Remove all old enodes from the updated canonical class
        let is_leaf = old_eclass_nodes.iter().all(|enode| enode.children().is_empty());  
        if !is_leaf {
            egraph[class].nodes.retain(|node| !old_eclass_nodes.contains(node));
        }
        vec![eclass]
    } else {
        vec![]
    }
}