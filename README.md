# Grammar

```console
Program 		-> FnDef Eof
FnDef   		-> "fn" Identifier "(" FnArgList ")" CompoundStmt
FnArgList 		-> "(" ")" % empty for now
CompoundStmt	-> "{" Statement "}"
Statement 		-> ReturnStmt 
                 | ";" % the empty statement
ReturnStmt  	-> "return" Integer ";"
```

This grammar accepts Rust-like function definitions:
```rust
fn foo() {
	return 4;
}
```
But this is obviously 10000 times worse than real Rust.
