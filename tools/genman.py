#!/usr/bin/env python3
"""genman.py - generate section-3 man pages for every public engine function.

Reads src/engine/*/*.h. For each prototype it takes:
  * a /** ... */ block immediately above     -> DESCRIPTION (+ @return, @see lines)
  * else the trailing /* ... */ on the line  -> one-line description
Also used by 'gge ls' to list functions.
"""
import argparse, glob, os, re, sys

HERE = os.path.dirname(os.path.abspath(__file__))
ENGINE = os.path.join(HERE, "..", "src", "engine")

PROTO = re.compile(r'^(?:static\s+inline\s+)?([A-Za-z_][\w\s\*]*?)\s*\b([a-z_][a-z0-9_]*)\s*\(([^;{]*)\)\s*(?:\{|;)', re.M)
DOCBLOCK = re.compile(r'/\*\*(.*?)\*/\s*$', re.S)
TRAIL = re.compile(r'/\*\s*(.*?)\s*\*/\s*$')

def first_sentence(s):
    s = s.splitlines()[0]
    m = re.match(r'(.+?\.)(\s|$)', s)
    return (m.group(1) if m else s).rstrip('.')

def esc(s):
    return s.replace('\\', '\\\\').replace('-', '\\-')

def scan():
    funcs = []
    for path in sorted(glob.glob(os.path.join(ENGINE, "*", "*.h"))):
        module = os.path.basename(os.path.dirname(path))
        header = os.path.basename(path)
        src = open(path, encoding="utf-8", errors="replace").read()
        for m in PROTO.finditer(src):
            ret, name, args = m.group(1).strip(), m.group(2), " ".join(m.group(3).split())
            if ret in ("return", "else", "if", "while", "for", "switch", "typedef") or name in ("if","for","while","switch","sizeof","return","defined"):
                continue
            if "(" in ret:                       # function pointer fields etc
                continue
            before = src[:m.start()]
            line_end = src.find("\n", m.end())
            line = src[m.start(): line_end if line_end > 0 else None]
            desc, ret_doc, see = "", "", []
            start = before.rfind("/**")
            end = before.rfind("*/")
            if start >= 0 and end > start and before[end + 2:].strip() == "":
                body = [re.sub(r'^\s*\*\s*', '', l) for l in before[start + 3:end].strip().splitlines()]
                for l in body:
                    if l.startswith("@return"): ret_doc = l[len("@return"):].strip()
                    elif l.startswith("@see"):   see = [s.strip() for s in l[len("@see"):].split(",")]
                    else: desc += l + "\n"
            else:
                t = TRAIL.search(line)
                if t: desc = t.group(1)
            funcs.append(dict(module=module, header=header, ret=ret, name=name, args=args,
                              desc=desc.strip(), ret_doc=ret_doc, see=see, inline="static inline" in line))
    # de-dup (same name in two headers, e.g. TTF/non-TTF variants)
    seen, out = set(), []
    for f in funcs:
        if f["name"] in seen: continue
        seen.add(f["name"]); out.append(f)
    return out

def page(f, siblings):
    n = f["name"]
    lines = [f'.TH {n.upper()} 3 "generalGameEngine" "Engine Manual" "Engine Manual"',
             ".SH NAME", f'{esc(n)} \\- {esc(first_sentence(f["desc"])) if f["desc"] else "engine function"}',
             ".SH SYNOPSIS", ".nf", f'.B #include \\(dq{f["header"]}\\(dq', f'.BI "{esc(f["ret"])} {esc(n)}({esc(f["args"])});"', ".fi",
             ".SH DESCRIPTION"]
    lines.append(esc(f["desc"]) if f["desc"] else "No description yet. Add a /** ... */ comment above the prototype in "
                 f"src/engine/{f['module']}/{f['header']} and run 'gge genman'.")
    if f["inline"]:
        lines += [".PP", "This is a static inline function; it is defined in the header."]
    if f["ret_doc"]:
        lines += [".SH RETURN VALUE", esc(f["ret_doc"])]
    lines += [".SH MODULE", f'{esc(f["module"])} (src/engine/{esc(f["module"])}/{esc(f["header"])})']
    see = f["see"] or [s["name"] for s in siblings if s["name"] != n][:8]
    if see:
        lines += [".SH SEE ALSO", ",\n".join(f'.BR {esc(s)} (3)' if "-" not in s else f'.BR {esc(s)} (7)' for s in see) + ","]
    lines += [".BR gge (1),", ".BR gge\\-scenes (7)"]
    return "\n".join(lines) + "\n"

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--out", help="write man3 pages here")
    ap.add_argument("--list", action="store_true")
    ap.add_argument("--module")
    ap.add_argument("--grep")
    a = ap.parse_args()
    funcs = scan()
    if a.module: funcs = [f for f in funcs if f["module"] == a.module]
    if a.grep:   funcs = [f for f in funcs if re.search(a.grep, f["name"])]
    if a.list or not a.out:
        w = max((len(f["name"]) for f in funcs), default=0)
        for f in funcs:
            print(f'{f["name"]:<{w}}  {f["module"]:<9} {f["desc"].splitlines()[0] if f["desc"] else ""}')
        return
    os.makedirs(a.out, exist_ok=True)
    bymod = {}
    for f in funcs: bymod.setdefault(f["module"], []).append(f)
    for f in funcs:
        open(os.path.join(a.out, f["name"] + ".3"), "w").write(page(f, bymod[f["module"]]))
    print(f"wrote {len(funcs)} pages to {a.out}")

if __name__ == "__main__":
    main()
