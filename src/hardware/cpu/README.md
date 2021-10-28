This directory contains code to implement CPU related structures.

`inst.c`

This is the parser for string instruction in such format: 

```
operator <SPACE> operand1 <COMMA & SPACE> operand2 <NEW LINE>
```

This parser do the parsing from left to right. It's exactly **One-Time Scanning** without any retrospect. So it's theoretically optimal (maybe some constant optimization). To implement this, we use **Trie** and **Deterministic Finite Automata** to compose a state machine at different level:

-   instruction level (DFA)
-   operand level (DFA)
-   effective address level (DFA)
-   number level (DFA), operator level (Trie) and register level (Trie)