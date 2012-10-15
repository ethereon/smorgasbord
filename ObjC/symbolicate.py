#!/usr/bin/env python

'''
    Symbolicate.py
    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    Symbolicator for Apple Crash Logs.

    :copyright: Copyright (c) 2012 Saumitro Dasgupta
    :license: MIT License [http://opensource.org/licenses/MIT]
'''

import os, sys, re
from argparse import ArgumentParser
from itertools import chain
from subprocess import check_output as run

def get_actual_dsym_binary(dsym_path):
    parts = run(['dwarfdump', '--file-stat', dsym_path]).replace('\n', ' ').split(' ')
    for p in parts:
        if p.startswith(dsym_path):
            return p
    return dsym_path

'''
Creates a lookup table based on UUID for dSYM files
The dict format is { uuid : (sym_path, dsym_arch) }
'''
def gen_dsym_map():
    dsyms = []
    for root, dirs, files in os.walk('.'):
        dsyms += [get_actual_dsym_binary(os.path.join(root, x)) for x in chain(dirs,files) if x.endswith('.dSYM')]
    get_uuid = lambda f: [v.split(' ')[1:] for v in run(['dwarfdump', '--uuid', f]).split('\n') if len(v)>0]
    return dict(((y[0], (x, y[1][1:-1])) for x in dsyms for y in get_uuid(x)))

def gen_binary_to_dsym_map(log_lines, dsym_map, include_shortnames=True):
    bin_map = {}
    bin_re = re.compile('(0x[0-9A-Fa-f]+) -\s*0x[0-9A-Fa-f]+ \+?(\S+).*<(.*)>.*')
    for x in log_lines:
        m = bin_re.match(x.strip())
        if m:
            (name, uuid, load_addr) = m.group(2, 3, 1)
            if uuid in dsym_map:
                dsym_info = list(dsym_map[uuid])+[load_addr]
                bin_map[name] = dsym_info
                if include_shortnames:
                    # Crash logs may use just Bar for identifying com.foo.Bar.
                    bin_map[name.split('.')[-1]] = dsym_info
    return bin_map

def resolve_dwarfdump(addr, dsym_info):  
    out = run(['dwarfdump', dsym_info[0], '--arch', dsym_info[1], '--lookup', addr])
    out = ' '.join(out.split('\n'))
    tags = ('AT_name', 'AT_decl_file', 'AT_decl_line')
    m = re.match('.*TAG_subprogram' + ''.join(['.*%s\( (.*?) \)'%x for x in tags]), out)
    if m:
        return '%s ( in %s : %s)'%m.group(1,2,3)
    print dsym_info
    return addr + ' [ unresolved ]'

def resolve_atos(addr, dsym_info):
    return run(('atos',)+sum(zip(['-o', '-arch', '-l'], dsym_info), ())+(addr,)).strip()

def is_unresolved(v):
    # Is this a hex address?
    if v.startswith('0x'):
        return True
    # Is this a relative offset?
    parts = v.split('+')
    return (len(parts)==2) and v[-1].isdigit()

def symbolicate(lines, dsym_map, resolver, dump_crash_dsyms, force):
    out_lines = []
    bin_dsym_map = gen_binary_to_dsym_map(lines, dsym_map)

    if dump_crash_dsyms:
        for bin_name in bin_dsym_map:
            info = bin_dsym_map[bin_name]
            sys.stderr.write('%40s\t%s (%s)\n'%(bin_name, info[0], info[1]))

    trace_re = re.compile('\d+\s+(\S+)\s+(\S+)\s+(.*)')
    for x in lines:
        m = trace_re.match(x)
        if m:
            (name, addr, offset_desc) = m.group(1,2,3)
            if (name in bin_dsym_map) and (force or is_unresolved(offset_desc)):            
                sym = resolver(addr, bin_dsym_map[name])
                x = x.replace(offset_desc, sym)
        out_lines.append(x)
    return out_lines

def main():
    # Parse the args
    parser = ArgumentParser(description = "Symbolicate OS X crash logs.")
    parser.add_argument('logfile', help = 'The crash log file to symbolicate.')
    parser.add_argument('-d', '--dwarfdump',
                        action = 'store_true',
                        help = 'Use dwarfdump instead of atos.')
    parser.add_argument('-f', '--force',
                        action = 'store_true',
                        help = 'Re-symbolicate already resolved symbols.')
    parser.add_argument('-s', '--logdsyms',
                        action = 'store_true',
                        help = 'Log list of detected dSym files to stderr.')
    args = parser.parse_args()

    # Locate dSyms
    dsym_map = gen_dsym_map()
    if len(dsym_map)==0:
        print "No dSYM files detected."
        sys.exit(1)

    # Symbolicate
    resolver = resolve_dwarfdump if args.dwarfdump else resolve_atos
    out_lines = symbolicate(lines = open(args.logfile).readlines(),
                            dsym_map = gen_dsym_map(),
                            resolver=resolver,
                            dump_crash_dsyms = args.logdsyms,
                            force = args.force)
    sys.stdout.writelines(out_lines)

if __name__=='__main__':
    main()