#!/usr/bin/env python3
"""genman.py - generate section-3 man pages for the public engine API.

Reads src/engine/*/*.h and writes one page per:
  * function   (a prototype, static inline or not)
  * type       (typedef struct / enum / plain typedef / function-pointer typedef)
  * macro      (#define with a doc block above it)
plus gge-api(7), an index of everything grouped by module.

Documentation comes from a /** ... */ block immediately above the declaration:
  * plain lines            -> DESCRIPTION
  * @return text           -> RETURN VALUE          (functions)
  * @field name text       -> FIELDS / VALUES       (types)
  * @see a, b, gge-topic   -> SEE ALSO  (names with a '-' are section 7 pages)
A prototype without a block falls back to its trailing /* ... */ comment.
Struct fields without an @field line fall back to their own trailing comment.

Also used by 'gge ls' to list functions and types.
"""
import argparse, glob, os, re, sys

HERE = os.path.dirname(os.path.abspath(__file__))
ENGINE = os.path.join(HERE, "..", "src", "engine")
TH = '"generalGameEngine" "Engine Manual" "Engine Manual"'

PROTO   = re.compile(r'^(?:static\s+inline\s+)?([A-Za-z_][\w\s\*]*?)\s*\b([a-z_][a-z0-9_]*)\s*\(([^;{]*)\)\s*(?:\{|;)', re.M)
TYPEDEF = re.compile(r'^typedef\s+(struct|enum)\s*(\w*)\s*\{(.*?)\}\s*(\w+)\s*;', re.M | re.S)
SIMPLE  = re.compile(r'^typedef\s+([^\n;{}()]+?)\s+(\w+)\s*;', re.M)
FNPTR   = re.compile(r'^typedef\s+([^\n;{}]*?)\(\s*\*\s*(\w+)\s*\)\s*\(([^\n;]*)\)\s*;', re.M)
DEFINE  = re.compile(r'^#define\s+([A-Za-z_]\w*)(\([^)]*\))?[ \t]*(.*)$', re.M)
TRAIL   = re.compile(r'/\*\s*(.*?)\s*\*/\s*$')

# ---------------------------------------------------------------- helpers

def first_sentence(s):
    """Text up to the first sentence-ending period, skipping abbreviations like e.g. and i.e."""
    s = s.splitlines()[0]
    for m in re.finditer(r'\.(\s|$)', s):
        head = s[:m.start()]
        if re.search(r'\b(e\.g|i\.e|etc|vs|cf)$', head): continue
        return head.rstrip('.')
    return s.rstrip('.')

def esc(s):
    """Escape text for troff: backslashes, hyphens, and control characters at line starts."""
    s = s.replace('\\', '\\\\').replace('-', '\\-')
    return "\n".join(('\\&' + l) if l[:1] in ".'" else l for l in s.splitlines())

def esc_code(s):
    """Escape verbatim code (inside .nf): backslashes and line-leading control characters only."""
    s = s.replace('\\', '\\\\')
    return "\n".join(('\\&' + l) if l[:1] in ".'" else l for l in s.splitlines())

def doc_block_before(src, pos):
    """The /** ... */ block that ends right before pos (only whitespace between), parsed."""
    before = src[:pos]
    start, end = before.rfind("/**"), before.rfind("*/")
    if start < 0 or end < start or before[end + 2:].strip() != "":
        return None
    body = [re.sub(r'^\s*\*\s*', '', l) for l in before[start + 3:end].strip().splitlines()]
    d = dict(desc="", ret="", see=[], fields=[])
    for l in body:
        if l.startswith("@return"):   d["ret"] = l[len("@return"):].strip()
        elif l.startswith("@see"):    d["see"] = [s.strip() for s in l[len("@see"):].split(",") if s.strip()]
        elif l.startswith("@field"):
            parts = l[len("@field"):].strip().split(None, 1)
            if parts: d["fields"].append((parts[0], parts[1] if len(parts) > 1 else ""))
        else: d["desc"] += l + "\n"
    d["desc"] = d["desc"].strip()
    return d

def strip_comments(s):
    return re.sub(r'/\*.*?\*/', '', s, flags=re.S)

def see_line(names):
    out = []
    for s in names:
        s = s.strip()
        if not s: continue
        sec = "7" if "-" in s else ("1" if s == "gge" else "3")
        out.append(f'.BR {esc(s)} ({sec})')
    return ",\n".join(out)

# ---------------------------------------------------------------- scanning

def headers():
    for path in sorted(glob.glob(os.path.join(ENGINE, "*", "*.h"))):
        yield (os.path.basename(os.path.dirname(path)), os.path.basename(path),
               open(path, encoding="utf-8", errors="replace").read())

def scan_functions():
    funcs = []
    for module, header, src in headers():
        for m in PROTO.finditer(src):
            ret, name, args = m.group(1).strip(), m.group(2), " ".join(m.group(3).split())
            if ret in ("return", "else", "if", "while", "for", "switch", "typedef") or name in ("if","for","while","switch","sizeof","return","defined"):
                continue
            if "(" in ret:                       # function pointer fields etc
                continue
            line_end = src.find("\n", m.end())
            line = src[m.start(): line_end if line_end > 0 else None]
            d = doc_block_before(src, m.start())
            if d is None:
                t = TRAIL.search(line)
                d = dict(desc=t.group(1) if t else "", ret="", see=[], fields=[])
            funcs.append(dict(kind="function", module=module, header=header, ret=ret, name=name, args=args,
                              desc=d["desc"], ret_doc=d["ret"], see=d["see"], inline="static inline" in line))
    return dedup(funcs)

def struct_fields(body):
    """[(name, trailing comment)] for each declarator in a struct body, line by line."""
    out = []
    for line in body.splitlines():
        t = TRAIL.search(line.strip())
        comment = t.group(1) if t else ""
        for stmt in strip_comments(line).split(";"):
            code = stmt.strip()
            if not code: continue
            # function pointer member: void (*on_enter)(Engine *, struct Scene *)
            fp = re.search(r'\(\s*\*\s*(\w+)\s*\)', code)
            if fp:
                out.append((fp.group(1), comment)); continue
            code = re.sub(r'\[[^\]]*\]', '', code)           # drop array sizes
            parts = code.split(",")
            first = parts[0].split()
            if not first: continue
            names = [first[-1].lstrip("*")] + [p.strip().lstrip("*").strip() for p in parts[1:]]
            for n in names:
                if re.match(r'^\w+$', n): out.append((n, comment))
    return out

def enum_values(body):
    out = []
    for line in body.splitlines():
        t = TRAIL.search(line.strip())
        comment = t.group(1) if t else ""
        for stmt in strip_comments(line).split(","):
            m = re.match(r'^(\w+)', stmt.strip())
            if m: out.append((m.group(1), comment))
    return out

def scan_types():
    types = []
    for module, header, src in headers():
        for m in TYPEDEF.finditer(src):
            kind, tag, body, name = m.group(1), m.group(2), m.group(3), m.group(4)
            d = doc_block_before(src, m.start()) or dict(desc="", ret="", see=[], fields=[])
            members = enum_values(body) if kind == "enum" else struct_fields(body)
            types.append(dict(kind=kind, module=module, header=header, name=name, code=m.group(0),
                              desc=d["desc"], see=d["see"], fields=d["fields"], members=members))
        for m in FNPTR.finditer(src):
            d = doc_block_before(src, m.start())
            if d is None: continue
            types.append(dict(kind="typedef", module=module, header=header, name=m.group(2), code=m.group(0),
                              desc=d["desc"], see=d["see"], fields=d["fields"], members=[]))
        for m in SIMPLE.finditer(src):
            if m.group(1).strip().startswith(("struct", "enum")) and "{" not in m.group(0):
                pass  # forward declaration or alias of a tagged type: fine, documented if it has a block
            d = doc_block_before(src, m.start())
            if d is None: continue
            line_end = src.find("\n", m.end())
            line = src[m.start(): line_end if line_end > 0 else None]
            types.append(dict(kind="typedef", module=module, header=header, name=m.group(2), code=line.strip(),
                              desc=d["desc"], see=d["see"], fields=d["fields"], members=[]))
    return dedup(types)

def scan_macros():
    macros = []
    for module, header, src in headers():
        for m in DEFINE.finditer(src):
            d = doc_block_before(src, m.start())
            if d is None: continue
            name, params, body = m.group(1), m.group(2) or "", m.group(3).strip()
            macros.append(dict(kind="macro", module=module, header=header, name=name, params=params, body=body,
                               desc=d["desc"], see=d["see"], ret_doc=d["ret"]))
    return dedup(macros)

def dedup(items):
    seen, out = set(), []
    for f in items:
        if f["name"] in seen: continue      # same name in two headers, e.g. TTF/non-TTF variants
        seen.add(f["name"]); out.append(f)
    return out

# ---------------------------------------------------------------- pages

def head(name, one_liner, header):
    return [f'.TH {name.upper()} 3 {TH}', ".SH NAME", f'{esc(name)} \\- {esc(one_liner)}',
            ".SH SYNOPSIS", ".nf", f'.B #include \\(dq{header}\\(dq']

def tail(f, see_default):
    lines = [".SH MODULE", f'{esc(f["module"])} (src/engine/{esc(f["module"])}/{esc(f["header"])})']
    see = f["see"] or see_default
    lines += [".SH SEE ALSO"]
    if see:
        lines += [see_line(see) + ","]
    lines += [".BR gge\\-api (7),", ".BR gge (1)"]
    return "\n".join(lines) + "\n"

def function_page(f, siblings):
    n = f["name"]
    lines = head(n, first_sentence(f["desc"]) if f["desc"] else "engine function", f["header"])
    lines += [f'.BI "{esc(f["ret"])} {esc(n)}({esc(f["args"])});"', ".fi", ".SH DESCRIPTION"]
    lines.append(esc(f["desc"]) if f["desc"] else "No description yet. Add a /** ... */ comment above the prototype in "
                 f"src/engine/{f['module']}/{f['header']} and run 'gge genman'.")
    if f["inline"]:
        lines += [".PP", "This is a static inline function; it is defined in the header."]
    if f["ret_doc"]:
        lines += [".SH RETURN VALUE", esc(f["ret_doc"])]
    return "\n".join(lines) + "\n" + tail(f, [s["name"] for s in siblings if s["name"] != n][:8])

def type_page(t, siblings):
    n = t["name"]
    what = {"struct": "struct", "enum": "enum", "typedef": "type"}[t["kind"]]
    lines = head(n, first_sentence(t["desc"]) if t["desc"] else f'engine {what}', t["header"])
    lines += [".ft B", esc_code(t["code"]), ".ft R", ".fi", ".SH DESCRIPTION"]
    lines.append(esc(t["desc"]) if t["desc"] else "No description yet. Add a /** ... */ comment above the typedef in "
                 f"src/engine/{t['module']}/{t['header']} and run 'gge genman'.")
    documented = dict(t["fields"])
    trailing = dict(t["members"])
    if t["members"] or t["fields"]:
        lines.append(".SH " + ("VALUES" if t["kind"] == "enum" else "FIELDS"))
        names = [m[0] for m in t["members"]]
        for fname, _ in t["fields"]:
            if fname not in names: names.append(fname)
        if not any(documented.get(n) or trailing.get(n) for n in names):
            lines += [".PP", esc(", ".join(names)) + "."]      # a plain list when nothing is described
        else:
            for fname in names:
                text = documented.get(fname) or trailing.get(fname) or ""
                lines += [".TP", f'.B {esc(fname)}', esc(text) if text else "(no description)"]
    return "\n".join(lines) + "\n" + tail(t, [s["name"] for s in siblings if s["name"] != n][:8])

def macro_page(m, siblings):
    n = m["name"]
    lines = head(n, first_sentence(m["desc"]) if m["desc"] else "engine macro", m["header"])
    lines += [".ft B", esc_code(f'#define {n}{m["params"]}' + (f' {m["body"]}' if m["body"] else "")), ".ft R", ".fi",
              ".SH DESCRIPTION", esc(m["desc"]) if m["desc"] else "No description yet."]
    if m["ret_doc"]:
        lines += [".SH RETURN VALUE", esc(m["ret_doc"])]
    return "\n".join(lines) + "\n" + tail(m, [s["name"] for s in siblings if s["name"] != n][:8])

def combined_page(items):
    """One page for several names that differ only by case: a NAME line naming all of them,
    every declaration in the SYNOPSIS, and a subsection per name."""
    names = [x["name"] for x in items]
    first = items[0]
    lines = [f'.TH {names[0].upper()} 3 {TH}', ".SH NAME",
             esc(", ".join(names)) + " \\- " + esc("; ".join(first_sentence(x["desc"]) if x["desc"] else x["name"] for x in items)),
             ".SH SYNOPSIS", ".nf", f'.B #include \\(dq{first["header"]}\\(dq']
    for x in items:
        if x["kind"] == "function":
            lines.append(f'.BI "{esc(x["ret"])} {esc(x["name"])}({esc(x["args"])});"')
        elif x["kind"] == "macro":
            lines += [".ft B", esc_code(f'#define {x["name"]}{x["params"]}' + (f' {x["body"]}' if x["body"] else "")), ".ft R"]
        else:
            lines += [".ft B", esc_code(x["code"]), ".ft R"]
    lines += [".fi", ".SH DESCRIPTION", "These names differ only in case and share this page."]
    see = []
    for x in items:
        what = {"function": "()", "macro": " (macro)"}.get(x["kind"], f' ({x["kind"]})')
        lines += [".SS " + esc(x["name"]) + what, esc(x["desc"]) if x["desc"] else "No description yet."]
        if x["kind"] == "function" and x.get("inline"):
            lines += [".PP", "This is a static inline function; it is defined in the header."]
        if x.get("ret_doc"):
            lines += [".PP", "Returns: " + esc(x["ret_doc"])]
        if x["kind"] in ("struct", "enum"):
            documented = dict(x["fields"]); trailing = dict(x["members"])
            fnames = [m[0] for m in x["members"]] + [n for n, _ in x["fields"] if n not in [m[0] for m in x["members"]]]
            for fname in fnames:
                text = documented.get(fname) or trailing.get(fname) or ""
                lines += [".TP", f'.B {esc(fname)}', esc(text) if text else "(no description)"]
        see += [s_ for s_ in x["see"] if s_ not in see]
    lines += [".SH MODULE", f'{esc(first["module"])} (src/engine/{esc(first["module"])}/{esc(first["header"])})', ".SH SEE ALSO"]
    if see: lines.append(see_line(see) + ",")
    lines += [".BR gge\\-api (7),", ".BR gge (1)"]
    return "\n".join(lines) + "\n"

def api_index(funcs, types, macros):
    """gge-api(7): every page, grouped by module, with one-line descriptions."""
    bymod = {}
    for x in types + macros + funcs:
        bymod.setdefault(x["module"], []).append(x)
    lines = [f'.TH GGE\\-API 7 {TH}', ".SH NAME", "gge\\-api \\- index of every engine function, type and macro by module",
             ".SH DESCRIPTION",
             "One line per public name. Each has its own section\\-3 page: \\fBgge man\\fR \\fIname\\fR. "
             "Every module also has a section\\-7 page describing how its pieces fit together: \\fBgge man\\fR \\fImodule\\fR.",
             ".PP", "Regenerate this page with \\fBgge genman\\fR."]
    for module in sorted(bymod):
        lines += [".SH " + module.upper(), f'src/engine/{module}/ \\(em see also \\fBgge\\-{module}\\fR(7).']
        kinds = [("struct", "Structs"), ("enum", "Enums"), ("typedef", "Types"), ("macro", "Macros"), ("function", "Functions")]
        for kind, title in kinds:
            items = [x for x in bymod[module] if x["kind"] == kind]
            if not items: continue
            lines += [".SS " + title]
            for x in items:
                one = first_sentence(x["desc"]) if x["desc"] else ""
                lines += [".TP", f'.BR {esc(x["name"])} (3)', esc(one) if one else "\\&"]
    lines += [".SH SEE ALSO", ".BR gge (1),", ".BR gge\\-overview (7)"]
    return "\n".join(lines) + "\n"

# ---------------------------------------------------------------- main

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--out", help="write man3 pages here (gge-api.7 goes to ../man7)")
    ap.add_argument("--list", action="store_true")
    ap.add_argument("--types", action="store_true", help="list types and macros instead of functions")
    ap.add_argument("--module")
    ap.add_argument("--grep")
    a = ap.parse_args()
    funcs, types, macros = scan_functions(), scan_types(), scan_macros()
    if a.module:
        funcs  = [f for f in funcs  if f["module"] == a.module]
        types  = [t for t in types  if t["module"] == a.module]
        macros = [m for m in macros if m["module"] == a.module]
    if a.grep:
        funcs  = [f for f in funcs  if re.search(a.grep, f["name"])]
        types  = [t for t in types  if re.search(a.grep, t["name"])]
        macros = [m for m in macros if re.search(a.grep, m["name"])]
    if a.list or not a.out:
        items = (types + macros) if a.types else funcs
        w = max((len(f["name"]) for f in items), default=0)
        for f in items:
            print(f"{f['name']:<{w}}  {f['module']:<9} {first_sentence(f['desc']) if f['desc'] else ''}")
        return
    os.makedirs(a.out, exist_ok=True)
    bymod, tymod, mmod = {}, {}, {}
    for f in funcs:  bymod.setdefault(f["module"], []).append(f)
    for t in types:  tymod.setdefault(t["module"], []).append(t)
    for m in macros: mmod.setdefault(m["module"], []).append(m)
    # names that differ only by case (Rect the struct, rect() the function) share one page,
    # because the default macOS filesystem would otherwise fold the two files into one
    groups = {}
    for x in funcs + types + macros: groups.setdefault(x["name"].lower(), []).append(x)
    for f in funcs:
        if len(groups[f["name"].lower()]) > 1: continue
        open(os.path.join(a.out, f["name"] + ".3"), "w").write(function_page(f, bymod[f["module"]]))
    for t in types:
        if len(groups[t["name"].lower()]) > 1: continue
        open(os.path.join(a.out, t["name"] + ".3"), "w").write(type_page(t, tymod[t["module"]]))
    for m in macros:
        if len(groups[m["name"].lower()]) > 1: continue
        open(os.path.join(a.out, m["name"] + ".3"), "w").write(macro_page(m, mmod[m["module"]]))
    for key, items in groups.items():
        if len(items) > 1:
            open(os.path.join(a.out, key + ".3"), "w").write(combined_page(items))
    man7 = os.path.join(os.path.dirname(os.path.abspath(a.out)), "man7")
    os.makedirs(man7, exist_ok=True)
    open(os.path.join(man7, "gge-api.7"), "w").write(api_index(funcs, types, macros))
    print(f"wrote {len(funcs)} function, {len(types)} type and {len(macros)} macro pages to {a.out}, gge-api.7 to {man7}")

if __name__ == "__main__":
    main()
