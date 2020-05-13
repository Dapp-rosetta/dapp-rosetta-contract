use std::collections::HashMap;
use borsh::{BorshDeserialize, BorshSerialize};
use near_bindgen::{
    env,
    ext_contract,
    near_bindgen,
    Promise,
};
use serde_json::json;

#[global_allocator]
static ALLOC: wee_alloc::WeeAlloc = wee_alloc::WeeAlloc::INIT;

#[near_bindgen]
#[derive(Default,BorshSerialize,BorshDeserialize)]
pub struct CounterContract{
    counter : i32
}

#[near_bindgen]
impl CounterContract{
    pub fn inc(&mut self) {
        self.counter+=1;
    }
    pub fn get(&self) -> i32{
        self.counter
    }
}