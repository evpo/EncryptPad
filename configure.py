#!/usr/bin/env python

import collections
import copy
import json
import sys
import os
import os.path
import platform
import re
import shlex
import shutil
import subprocess
import traceback
import logging
import time
import errno
import optparse # pylint: disable=deprecated-module

# An error caused by and to be fixed by the user, e.g. invalid command line argument
class UserError(Exception):
    pass


# An error caused by bugs in this script or when reading/parsing build data files
# Those are not expected to be fixed by the user of this script
class InternalError(Exception):
    pass

def flatten(l):
    return sum(l, [])

def normalize_source_path(source):
    """
    cmake needs this, and nothing else minds
    """
    return os.path.normpath(source).replace('\\', '/')

class LexResult(object):
    pass

class LexerError(InternalError):
    def __init__(self, msg, lexfile, line):
        super(LexerError, self).__init__(msg)
        self.msg = msg
        self.lexfile = lexfile
        self.line = line

    def __str__(self):
        return '%s at %s:%d' % (self.msg, self.lexfile, self.line)

def parse_lex_dict(as_list):
    if len(as_list) % 3 != 0:
        raise InternalError("Lex dictionary has invalid format (input not divisible by 3): %s" % as_list)

    result = {}
    for key, sep, value in [as_list[3*i:3*i+3] for i in range(0, len(as_list)//3)]:
        if sep != '->':
            raise InternalError("Lex dictionary has invalid format")
        result[key] = value
    return result

def lex_me_harder(infofile, allowed_groups, allowed_maps, name_val_pairs):
    """
    Generic lexer function for info.txt and src/build-data files
    """
    out = LexResult()

    # Format as a nameable Python variable
    def py_var(group):
        return group.replace(':', '_')

    lexer = shlex.shlex(open(infofile), infofile, posix=True)
    lexer.wordchars += ':.<>/,-!?+*' # handle various funky chars in info.txt

    groups = allowed_groups + allowed_maps
    for group in groups:
        out.__dict__[py_var(group)] = []
    for (key, val) in name_val_pairs.items():
        out.__dict__[key] = val

    def lexed_tokens(): # Convert to an iterator
        while True:
            token = lexer.get_token()
            if token != lexer.eof:
                yield token
            else:
                return

    for token in lexed_tokens():
        match = re.match('<(.*)>', token)

        # Check for a grouping
        if match is not None:
            group = match.group(1)

            if group not in groups:
                raise LexerError('Unknown group "%s"' % (group),
                                 infofile, lexer.lineno)

            end_marker = '</' + group + '>'

            token = lexer.get_token()
            while token != end_marker:
                out.__dict__[py_var(group)].append(token)
                token = lexer.get_token()
                if token is None:
                    raise LexerError('Group "%s" not terminated' % (group),
                                     infofile, lexer.lineno)

        elif token in name_val_pairs.keys():
            if isinstance(out.__dict__[token], list):
                out.__dict__[token].append(lexer.get_token())
            else:
                out.__dict__[token] = lexer.get_token()

        else: # No match -> error
            raise LexerError('Bad token "%s"' % (token), infofile, lexer.lineno)

    for group in allowed_maps:
        out.__dict__[group] = parse_lex_dict(out.__dict__[group])

    return out

class InfoObject(object):
    def __init__(self, infofile):
        """
        Constructor sets members `infofile`, `lives_in`, `parent_module` and `basename`
        """

        self.infofile = infofile
        (dirname, basename) = os.path.split(infofile)
        self.lives_in = dirname
        if basename == 'info.txt':
            (obj_dir, self.basename) = os.path.split(dirname)
            if os.access(os.path.join(obj_dir, 'info.txt'), os.R_OK):
                self.parent_module = os.path.basename(obj_dir)
            else:
                self.parent_module = None
        else:
            self.basename = basename.replace('.txt', '')


class ModuleInfo(InfoObject):
    """
    Represents the information about a particular module
    """

    def __init__(self, infofile):
        super(ModuleInfo, self).__init__(infofile)
        lex = lex_me_harder(
            infofile,
            ['header:internal', 'header:public', 'header:external', 'requires',
             'os_features', 'arch', 'cc', 'libs', 'frameworks', 'comment', 'warning'
            ],
            ['defines'],
            {
                'load_on': 'auto',
                'need_isa': '',
                'third_party': 'no'
            })

        def check_header_duplicates(header_list_public, header_list_internal):
            pub_header = set(header_list_public)
            int_header = set(header_list_internal)
            if not pub_header.isdisjoint(int_header):
                logging.error("Module %s header contains same header in public and internal sections" % self.infofile)

        check_header_duplicates(lex.header_public, lex.header_internal)

        all_source_files = []
        all_header_files = []

        for fspath in os.listdir(self.lives_in):
            if fspath.endswith('.cpp'):
                all_source_files.append(fspath)
            elif fspath.endswith('.h'):
                all_header_files.append(fspath)

        self.source = all_source_files

        # If not entry for the headers, all are assumed public
        if lex.header_internal == [] and lex.header_public == []:
            self.header_public = list(all_header_files)
            self.header_internal = []
        else:
            self.header_public = lex.header_public
            self.header_internal = lex.header_internal
        self.header_external = lex.header_external

        # Coerce to more useful types
        def convert_lib_list(l):
            if len(l) % 3 != 0:
                raise InternalError("Bad <libs> in module %s" % (self.basename))
            result = {}

            for sep in l[1::3]:
                if sep != '->':
                    raise InternalError("Bad <libs> in module %s" % (self.basename))

            for (targetlist, vallist) in zip(l[::3], l[2::3]):
                vals = vallist.split(',')
                for target in targetlist.split(','):
                    result[target] = result.setdefault(target, []) + vals
            return result

        # Convert remaining lex result to members
        self.arch = lex.arch
        self.cc = lex.cc
        self.comment = ' '.join(lex.comment) if lex.comment else None
        self._defines = lex.defines
        self._validate_defines_content(self._defines)
        self.frameworks = convert_lib_list(lex.frameworks)
        self.libs = convert_lib_list(lex.libs)
        self.load_on = lex.load_on
        self.need_isa = lex.need_isa.split(',') if lex.need_isa else []
        self.third_party = True if lex.third_party == 'yes' else False
        self.os_features = lex.os_features
        self.requires = lex.requires
        self.warning = ' '.join(lex.warning) if lex.warning else None

        # Modify members
        self.source = [normalize_source_path(os.path.join(self.lives_in, s)) for s in self.source]
        self.header_internal = [os.path.join(self.lives_in, s) for s in self.header_internal]
        self.header_public = [os.path.join(self.lives_in, s) for s in self.header_public]
        self.header_external = [os.path.join(self.lives_in, s) for s in self.header_external]

        # Filesystem read access check
        for src in self.source + self.header_internal + self.header_public + self.header_external:
            if not os.access(src, os.R_OK):
                logging.error("Missing file %s in %s" % (src, infofile))

        # Check for duplicates
        def intersect_check(type_a, list_a, type_b, list_b):
            intersection = set.intersection(set(list_a), set(list_b))
            if intersection:
                logging.error('Headers %s marked both %s and %s' % (' '.join(intersection), type_a, type_b))

        intersect_check('public', self.header_public, 'internal', self.header_internal)
        intersect_check('public', self.header_public, 'external', self.header_external)
        intersect_check('external', self.header_external, 'internal', self.header_internal)

    @staticmethod
    def _validate_defines_content(defines):
        for key, value in defines.items():
            if not re.match('^[0-9A-Za-z_]{3,30}$', key):
                raise InternalError('Module defines key has invalid format: "%s"' % key)
            if not re.match('^[0-9]{8}$', value):
                raise InternalError('Module defines value has invalid format: "%s"' % value)

    def cross_check(self, arch_info, cc_info, all_os_features):

        for feat in set(flatten([o.split(',') for o in self.os_features])):
            if feat not in all_os_features:
                logging.error("Module %s uses an OS feature (%s) which no OS supports", self.infofile, feat)

        for supp_cc in self.cc:
            if supp_cc not in cc_info:
                colon_idx = supp_cc.find(':')
                # a versioned compiler dependency
                if colon_idx > 0 and supp_cc[0:colon_idx] in cc_info:
                    pass
                else:
                    raise InternalError('Module %s mentions unknown compiler %s' % (self.infofile, supp_cc))
        for supp_arch in self.arch:
            if supp_arch not in arch_info:
                raise InternalError('Module %s mentions unknown arch %s' % (self.infofile, supp_arch))

    def sources(self):
        return self.source

    def public_headers(self):
        return self.header_public

    def internal_headers(self):
        return self.header_internal

    def external_headers(self):
        return self.header_external

    def defines(self):
        return [(key + ' ' + value) for key, value in self._defines.items()]

    def compatible_cpu(self, archinfo, options):
        arch_name = archinfo.basename
        cpu_name = options.cpu

        for isa in self.need_isa:
            if isa in options.disable_intrinsics:
                return False # explicitly disabled

            if isa not in archinfo.isa_extensions:
                return False

        if self.arch != []:
            if arch_name not in self.arch and cpu_name not in self.arch:
                return False

        return True

    def compatible_os(self, os_data, options):
        if not self.os_features:
            return True

        def has_all(needed, provided):
            for n in needed:
                if n not in provided:
                    return False
            return True

        provided_features = os_data.enabled_features(options)

        for feature_set in self.os_features:
            if has_all(feature_set.split(','), provided_features):
                return True

        return False

    def compatible_compiler(self, ccinfo, cc_min_version, arch):
        # Check if this compiler supports the flags we need
        def supported_isa_flags(ccinfo, arch):
            for isa in self.need_isa:
                if ccinfo.isa_flags_for(isa, arch) is None:
                    return False
            return True

        # Check if module gives explicit compiler dependencies
        def supported_compiler(ccinfo, cc_min_version):
            if self.cc == []:
                # no compiler restriction
                return True

            if ccinfo.basename in self.cc:
                # compiler is supported, independent of version
                return True

            # Maybe a versioned compiler dep
            for cc in self.cc:
                try:
                    name, version = cc.split(":")
                    if name == ccinfo.basename:
                        min_cc_version = [int(v) for v in version.split('.')]
                        cur_cc_version = [int(v) for v in cc_min_version.split('.')]
                        # With lists of ints, this does what we want
                        return cur_cc_version >= min_cc_version
                except ValueError:
                    # No version part specified
                    pass

            return False # compiler not listed

        return supported_isa_flags(ccinfo, arch) and supported_compiler(ccinfo, cc_min_version)

    def dependencies(self, osinfo):
        # base is an implicit dep for all submodules
        deps = ['base']
        if self.parent_module != None:
            deps.append(self.parent_module)

        for req in self.requires:
            if req.find('?') != -1:
                (cond, dep) = req.split('?')
                if osinfo is None or cond in osinfo.target_features:
                    deps.append(dep)
            else:
                deps.append(req)

        return deps

    def dependencies_exist(self, modules):
        """
        Ensure that all dependencies of this module actually exist, warning
        about any that do not
        """

        missing = [s for s in self.dependencies(None) if s not in modules]

        if missing:
            logging.error("Module '%s', dep of '%s', does not exist" % (
                missing, self.basename))

def load_info_files(search_dir, descr, filename_matcher, class_t):
    info = {}

    def filename_matches(filename):
        if isinstance(filename_matcher, str):
            return filename == filename_matcher
        else:
            return filename_matcher.match(filename) is not None

    for (dirpath, _, filenames) in os.walk(search_dir):
        for filename in filenames:
            filepath = os.path.join(dirpath, filename)
            if filename_matches(filename):
                info_obj = class_t(filepath)
                info[info_obj.basename] = info_obj

    if info:
        infotxt_basenames = ' '.join(sorted([key for key in info]))
        logging.debug('Loaded %d %s files: %s' % (len(info), descr, infotxt_basenames))
    else:
        logging.warning('Failed to load any %s files' % (descr))

    return info

def load_build_data_info_files(source_paths, descr, subdir, class_t):
    matcher = re.compile(r'[_a-z0-9]+\.txt$')
    return load_info_files(os.path.join(source_paths.build_data_dir, subdir), descr, matcher, class_t)

def portable_symlink(file_path, target_dir, method):
    """
    Copy or link the file, depending on what the platform offers
    """

    if not os.access(file_path, os.R_OK):
        logging.warning('Missing file %s' % (file_path))
        return

    if method == 'symlink':
        rel_file_path = os.path.relpath(file_path, start=target_dir)
        os.symlink(rel_file_path, os.path.join(target_dir, os.path.basename(file_path)))
    elif method == 'hardlink':
        os.link(file_path, os.path.join(target_dir, os.path.basename(file_path)))
    elif method == 'copy':
        shutil.copy(file_path, target_dir)
    else:
        raise UserError('Unknown link method %s' % (method))

def choose_link_method(options):
    """
    Choose the link method based on system availability and user request
    """

    req = options.link_method

    def useable_methods():
        # Symbolic link support on Windows was introduced in Windows 6.0 (Vista) and Python 3.2
        # Furthermore the SeCreateSymbolicLinkPrivilege is required in order to successfully create symlinks
        # So only try to use symlinks on Windows if explicitly requested
        if req == 'symlink' and options.os == 'windows':
            yield 'symlink'
        # otherwise keep old conservative behavior
        if 'symlink' in os.__dict__ and options.os != 'windows':
            yield 'symlink'
        if 'link' in os.__dict__:
            yield 'hardlink'
        yield 'copy'

    for method in useable_methods():
        if req is None or req == method:
            logging.info('Using %s to link files into build dir ' \
                         '(use --link-method to change)' % (method))
            return method

    logging.warning('Could not use link method "%s", will copy instead' % (req))
    return 'copy'

def system_cpu_info():

    cpu_info = []

    if platform.machine() != '':
        cpu_info.append(platform.machine())

    if platform.processor() != '':
        cpu_info.append(platform.processor())

    if 'uname' in os.__dict__:
        cpu_info.append(os.uname()[4])

    return cpu_info

def canon_processor(archinfo, proc):
    proc = fixup_proc_name(proc)

    # First, try to search for an exact match
    for ainfo in archinfo.values():
        if ainfo.basename == proc or proc in ainfo.aliases:
            return ainfo.basename

    return None

def guess_processor(archinfo):
    for info_part in system_cpu_info():
        if info_part:
            match = canon_processor(archinfo, info_part)
            if match != None:
                logging.debug("Matched '%s' to processor '%s'" % (info_part, match))
                return match, info_part
            else:
                logging.debug("Failed to deduce CPU from '%s'" % info_part)

    raise UserError('Could not determine target CPU; set with --cpu')

def have_program(program):
    """
    Test for the existence of a program
    """

    def exe_test(path, program):
        exe_file = os.path.join(path, program)

        if os.path.exists(exe_file) and os.access(exe_file, os.X_OK):
            logging.debug('Found program %s in %s' % (program, path))
            return True
        else:
            return False

    exe_suffixes = ['', '.exe']

    for path in os.environ['PATH'].split(os.pathsep):
        for suffix in exe_suffixes:
            if exe_test(path, program + suffix):
                return True

    logging.debug('Program %s not found' % (program))
    return False

def set_defaults_for_unset_options(options, info_arch, info_cc): # pylint: disable=too-many-branches
    if options.os is None:
        system_from_python = platform.system().lower()
        if re.match('^cygwin_.*', system_from_python):
            logging.debug("Converting '%s' to 'cygwin'", system_from_python)
            options.os = 'cygwin'
        else:
            options.os = system_from_python
        logging.info('Guessing target OS is %s (use --os to set)' % (options.os))

    def deduce_compiler_type_from_cc_bin(cc_bin):
        if cc_bin.find('clang') != -1 or cc_bin in ['emcc', 'em++']:
            return 'clang'
        if cc_bin.find('-g++') != -1:
            return 'gcc'
        return None

    if options.compiler is None and options.compiler_binary != None:
        options.compiler = deduce_compiler_type_from_cc_bin(options.compiler_binary)

    if options.compiler is None:
        if options.os == 'windows':
            if have_program('g++') and not have_program('cl'):
                options.compiler = 'gcc'
            else:
                options.compiler = 'msvc'
        elif options.os in ['darwin', 'freebsd', 'openbsd', 'ios']:
            # Prefer Clang on these systems
            if have_program('clang++'):
                options.compiler = 'clang'
            else:
                options.compiler = 'gcc'
                if options.os == 'openbsd':
                    # The assembler shipping with OpenBSD 5.9 does not support avx2
                    del info_cc['gcc'].isa_flags['avx2']
        else:
            options.compiler = 'gcc'

        if options.compiler is None:
            logging.error('Could not guess which compiler to use, use --cc or CXX to set')
        else:
            logging.info('Guessing to use compiler %s (use --cc or CXX to set)' % (options.compiler))

    if options.cpu is None:
        (arch, cpu) = guess_processor(info_arch)
        options.arch = arch
        options.cpu = cpu
        if is_windows(options):
            options.cpu = "x86_32"
        logging.info('Guessing target processor is a %s (use --cpu to set)' % (options.arch))

    if is_windows(options):
        options.build_botan = True
        options.build_zlib = True
        options.build_bzip2 = True

class EncryptPadConfigureLogHandler(logging.StreamHandler, object):
    def emit(self, record):
        # Do the default stuff first
        super(EncryptPadConfigureLogHandler, self).emit(record)
        # Exit script if and ERROR or worse occurred
        if record.levelno >= logging.ERROR:
            sys.exit(1)

def setup_logging(options):
    log_level = logging.INFO

    lh = EncryptPadConfigureLogHandler(sys.stdout)
    lh.setFormatter(logging.Formatter('%(levelname) 7s: %(message)s'))
    logging.getLogger().addHandler(lh)
    logging.getLogger().setLevel(log_level)

# Workaround for Windows systems where antivirus is enabled GH #353
def robust_rmtree(path, max_retries=5):
    for _ in range(max_retries):
        try:
            shutil.rmtree(path)
            return
        except OSError:
            time.sleep(0.1)

    # Final attempt, pass any exceptions up to caller.
    shutil.rmtree(path)

# Workaround for Windows systems where antivirus is enabled GH #353
def robust_makedirs(directory, max_retries=5):
    for _ in range(max_retries):
        try:
            os.makedirs(directory)
            return
        except OSError as e:
            if e.errno == errno.EEXIST:
                raise
            else:
                time.sleep(0.1)

    # Final attempt, pass any exceptions up to caller.
    os.makedirs(directory)

def read_textfile(filepath):
    """
    Read a whole file into memory as a string
    """
    if filepath is None:
        return ''

    with open(filepath) as f:
        return ''.join(f.readlines())


def process_template(template_file, variables):
    # pylint: disable=too-many-branches,too-many-statements

    """
    Perform template substitution

    The template language supports (un-nested) conditionals.
    """
    class SimpleTemplate(object):

        def __init__(self, vals):
            self.vals = vals
            self.value_pattern = re.compile(r'%{([a-z][a-z_0-9\|]+)}')
            self.cond_pattern = re.compile('%{(if|unless) ([a-z][a-z_0-9]+)}')
            self.for_pattern = re.compile('(.*)%{for ([a-z][a-z_0-9]+)}')
            self.join_pattern = re.compile('(.*)%{join ([a-z][a-z_0-9]+)}')

        def substitute(self, template):
            # pylint: disable=too-many-locals
            def insert_value(match):
                v = match.group(1)
                if v in self.vals:
                    return str(self.vals.get(v))
                if v.endswith('|upper'):
                    v = v.replace('|upper', '')
                    if v in self.vals:
                        return str(self.vals.get(v)).upper()

                raise KeyError(v)

            lines = template.splitlines()

            output = ""
            idx = 0

            while idx < len(lines):
                cond_match = self.cond_pattern.match(lines[idx])
                join_match = self.join_pattern.match(lines[idx])
                for_match = self.for_pattern.match(lines[idx])

                if cond_match:
                    cond_type = cond_match.group(1)
                    cond_var = cond_match.group(2)

                    include_cond = False

                    if cond_type == 'if' and cond_var in self.vals and self.vals.get(cond_var):
                        include_cond = True
                    elif cond_type == 'unless' and (cond_var not in self.vals or (not self.vals.get(cond_var))):
                        include_cond = True

                    idx += 1
                    while idx < len(lines):
                        if lines[idx] == '%{endif}':
                            break
                        if include_cond:
                            output += lines[idx] + "\n"
                        idx += 1
                elif join_match:
                    join_var = join_match.group(2)
                    join_str = ' '
                    join_line = '%%{join %s}' % (join_var)
                    output += lines[idx].replace(join_line, join_str.join(self.vals[join_var])) + "\n"
                elif for_match:
                    for_prefix = for_match.group(1)
                    output += for_prefix
                    for_var = for_match.group(2)

                    if for_var not in self.vals:
                        raise InternalError("Unknown for loop iteration variable '%s'" % (for_var))

                    var = self.vals[for_var]
                    if not isinstance(var, list):
                        raise InternalError("For loop iteration variable '%s' is not a list" % (for_var))
                    idx += 1

                    for_body = ""
                    while idx < len(lines):
                        if lines[idx] == '%{endfor}':
                            break
                        for_body += lines[idx] + "\n"
                        idx += 1

                    for v in var:
                        if isinstance(v, dict):
                            for_val = for_body
                            for ik, iv in v.items():
                                for_val = for_val.replace('%{' + ik + '}', iv)
                            output += for_val + "\n"
                        else:
                            output += for_body.replace('%{i}', v).replace('%{i|upper}', v.upper())
                    output += "\n"
                else:
                    output += lines[idx] + "\n"
                idx += 1

            return self.value_pattern.sub(insert_value, output) + '\n'

    try:
        return SimpleTemplate(variables).substitute(read_textfile(template_file))
    except KeyError as e:
        logging.error('Unbound var %s in template %s' % (e, template_file))
    except Exception as e: # pylint: disable=broad-except
        logging.error('Exception %s during template processing file %s' % (e, template_file))

def fixup_proc_name(proc):
    proc = proc.lower().replace(' ', '')
    for junk in ['(tm)', '(r)']:
        proc = proc.replace(junk, '')
    return proc

class ArchInfo(InfoObject):
    def __init__(self, infofile):
        super(ArchInfo, self).__init__(infofile)
        lex = lex_me_harder(
            infofile,
            ['aliases', 'isa_extensions'],
            [],
            {
                'endian': None,
                'family': None,
                'wordsize': 32
            })

        self.aliases = lex.aliases
        self.endian = lex.endian
        self.family = lex.family
        self.isa_extensions = lex.isa_extensions
        self.wordsize = int(lex.wordsize)

        if self.wordsize not in [32, 64]:
            logging.error('Unexpected wordsize %d for arch %s', self.wordsize, infofile)

        alphanumeric = re.compile('^[a-z0-9]+$')
        for isa in self.isa_extensions:
            if alphanumeric.match(isa) is None:
                logging.error('Invalid name for ISA extension "%s"', isa)

    def supported_isa_extensions(self, cc, options):
        isas = []

        for isa in self.isa_extensions:
            if isa not in options.disable_intrinsics:
                if cc.isa_flags_for(isa, self.basename) is not None:
                    isas.append(isa)

        return sorted(isas)

class OsInfo(InfoObject): # pylint: disable=too-many-instance-attributes
    def __init__(self, infofile):
        super(OsInfo, self).__init__(infofile)
        lex = lex_me_harder(
            infofile,
            ['aliases', 'target_features'],
            [],
            {
                'program_suffix': '',
                'obj_suffix': 'o',
                'soname_suffix': '',
                'soname_pattern_patch': '',
                'soname_pattern_abi': '',
                'soname_pattern_base': '',
                'static_suffix': 'a',
                'ar_command': 'ar',
                'ar_options': '',
                'ar_output_to': '',
                'install_root': '/usr/local',
                'header_dir': 'include',
                'bin_dir': 'bin',
                'lib_dir': 'lib',
                'doc_dir': 'share/doc',
                'man_dir': 'share/man',
                'use_stack_protector': 'true',
                'so_post_link_command': '',
                'cli_exe_name': 'encryptcli',
                'test_exe_name': 'encryptpad_test',
                'lib_prefix': 'lib',
                'library_name': 'botan{suffix}-{major}',
            })

        if lex.ar_command == 'ar' and lex.ar_options == '':
            lex.ar_options = 'crs'

        if lex.soname_pattern_base:
            self.soname_pattern_base = lex.soname_pattern_base
            if lex.soname_pattern_patch == '' and lex.soname_pattern_abi == '':
                self.soname_pattern_patch = lex.soname_pattern_base
                self.soname_pattern_abi = lex.soname_pattern_base
            elif lex.soname_pattern_abi != '' and lex.soname_pattern_abi != '':
                self.soname_pattern_patch = lex.soname_pattern_patch
                self.soname_pattern_abi = lex.soname_pattern_abi
            else:
                # base set, only one of patch/abi set
                raise InternalError("Invalid soname_patterns in %s" % (self.infofile))
        else:
            if lex.soname_suffix:
                self.soname_pattern_base = "libbotan{lib_suffix}-{version_major}.%s" % (lex.soname_suffix)
                self.soname_pattern_abi = self.soname_pattern_base + ".{abi_rev}"
                self.soname_pattern_patch = self.soname_pattern_abi + ".{version_minor}.{version_patch}"
            else:
                # Could not calculate soname_pattern_*
                # This happens for OSs without shared library support (e.g. nacl, mingw, includeos, cygwin)
                self.soname_pattern_base = None
                self.soname_pattern_abi = None
                self.soname_pattern_patch = None

        self._aliases = lex.aliases
        self.ar_command = lex.ar_command
        self.ar_options = lex.ar_options
        self.bin_dir = lex.bin_dir
        self.cli_exe_name = lex.cli_exe_name
        self.test_exe_name = lex.test_exe_name
        self.doc_dir = lex.doc_dir
        self.header_dir = lex.header_dir
        self.install_root = lex.install_root
        self.lib_dir = lex.lib_dir
        self.lib_prefix = lex.lib_prefix
        self.library_name = lex.library_name
        self.man_dir = lex.man_dir
        self.obj_suffix = lex.obj_suffix
        self.program_suffix = lex.program_suffix
        self.so_post_link_command = lex.so_post_link_command
        self.static_suffix = lex.static_suffix
        self.target_features = lex.target_features
        self.use_stack_protector = (lex.use_stack_protector == "true")

    def matches_name(self, nm):
        if nm in self._aliases:
            return True

        for alias in self._aliases:
            if re.match(alias, nm):
                return True
        return False

    def building_shared_supported(self):
        return self.soname_pattern_base != None

    def enabled_features(self, options):
        feats = []
        for feat in self.target_features:
            if feat not in options.without_os_features:
                feats.append(feat)
        for feat in options.with_os_features:
            if feat not in self.target_features:
                feats.append(feat)

        return sorted(feats)

class CompilerInfo(InfoObject): # pylint: disable=too-many-instance-attributes
    def __init__(self, infofile):
        super(CompilerInfo, self).__init__(infofile)
        lex = lex_me_harder(
            infofile,
            [],
            ['cpu_flags', 'cpu_flags_no_debug', 'so_link_commands', 'binary_link_commands',
             'mach_abi_linking', 'isa_flags', 'sanitizers'],
            {
                'binary_name': None,
                'linker_name': None,
                'macro_name': None,
                'output_to_object': '-o ',
                'output_to_exe': '-o ',
                'add_include_dir_option': '-I',
                'add_lib_dir_option': '-L',
                'add_sysroot_option': '',
                'add_lib_option': '-l',
                'add_framework_option': '-framework ',
                'preproc_flags': '-E',
                'compile_flags': '-c',
                'debug_info_flags': '-g',
                'optimization_flags': '',
                'size_optimization_flags': '',
                'sanitizer_optimization_flags': '',
                'coverage_flags': '',
                'stack_protector_flags': '',
                'shared_flags': '',
                'lang_flags': '',
                'warning_flags': '',
                'maintainer_warning_flags': '',
                'visibility_build_flags': '',
                'visibility_attribute': '',
                'ar_command': '',
                'ar_options': '',
                'ar_output_to': '',
                'static_linking': '',
                'large_files': '',
            })

        self.add_framework_option = lex.add_framework_option
        self.add_include_dir_option = lex.add_include_dir_option
        self.add_lib_dir_option = lex.add_lib_dir_option
        self.add_lib_option = lex.add_lib_option
        self.add_sysroot_option = lex.add_sysroot_option
        self.ar_command = lex.ar_command
        self.ar_options = lex.ar_options
        self.ar_output_to = lex.ar_output_to
        self.binary_link_commands = lex.binary_link_commands
        self.binary_name = lex.binary_name
        self.cpu_flags = lex.cpu_flags
        self.cpu_flags_no_debug = lex.cpu_flags_no_debug
        self.compile_flags = lex.compile_flags
        self.coverage_flags = lex.coverage_flags
        self.debug_info_flags = lex.debug_info_flags
        self.isa_flags = lex.isa_flags
        self.lang_flags = lex.lang_flags
        self.linker_name = lex.linker_name
        self.mach_abi_linking = lex.mach_abi_linking
        self.macro_name = lex.macro_name
        self.maintainer_warning_flags = lex.maintainer_warning_flags
        self.optimization_flags = lex.optimization_flags
        self.output_to_exe = lex.output_to_exe
        self.output_to_object = lex.output_to_object
        self.preproc_flags = lex.preproc_flags
        self.sanitizers = lex.sanitizers
        self.sanitizer_types = []
        self.sanitizer_optimization_flags = lex.sanitizer_optimization_flags
        self.shared_flags = lex.shared_flags
        self.size_optimization_flags = lex.size_optimization_flags
        self.so_link_commands = lex.so_link_commands
        self.stack_protector_flags = lex.stack_protector_flags
        self.visibility_attribute = lex.visibility_attribute
        self.visibility_build_flags = lex.visibility_build_flags
        self.warning_flags = lex.warning_flags
        self.static_linking = lex.static_linking
        self.large_files = lex.large_files

    def isa_flags_for(self, isa, arch):
        if isa in self.isa_flags:
            return self.isa_flags[isa]
        arch_isa = '%s:%s' % (arch, isa)
        if arch_isa in self.isa_flags:
            return self.isa_flags[arch_isa]
        return None

    def get_isa_specific_flags(self, isas, arch, options):
        flags = set()

        def simd32_impl():
            for simd_isa in ['sse2', 'altivec', 'neon']:
                if simd_isa in arch.isa_extensions and \
                   simd_isa not in options.disable_intrinsics and \
                   self.isa_flags_for(simd_isa, arch.basename):
                    return simd_isa
            return None

        for isa in isas:

            if isa == 'simd':
                isa = simd32_impl()

                if isa is None:
                    continue

            flagset = self.isa_flags_for(isa, arch.basename)
            if flagset is None:
                raise UserError('Compiler %s does not support %s' % (self.basename, isa))
            flags.add(flagset)

        return " ".join(sorted(flags))

    def gen_shared_flags(self, options):
        """
        Return the shared library build flags, if any
        """

        def flag_builder():
            return ''

        return ' '.join(list(flag_builder()))

    def gen_ldflags(self, options):
        """
        Return the general linking flags, if any
        """
        def flag_builder():
            if options.static_linking:
                yield self.static_linking

        return ' '.join(list(flag_builder()))

    def gen_visibility_attribute(self, options):
        return ''

    def mach_abi_link_flags(self, options, with_debug_info=None):
        #pylint: disable=too-many-branches

        """
        Return the machine specific ABI flags
        """

        if with_debug_info is None:
            with_debug_info = options.with_debug_info

        def mach_abi_groups():

            yield 'all'

            if with_debug_info:
                yield 'rt-debug'
            else:
                yield 'rt'

            for all_except in [s for s in self.mach_abi_linking.keys() if s.startswith('all!')]:
                exceptions = all_except[4:].split(',')
                if options.os not in exceptions and options.arch not in exceptions:
                    yield all_except

            yield options.os
            yield options.cpu

        abi_link = set()
        for what in mach_abi_groups():
            if what in self.mach_abi_linking:
                flag = self.mach_abi_linking.get(what)
                if flag != None and flag != '' and flag not in abi_link:
                    abi_link.add(flag)

        if options.with_stack_protector and self.stack_protector_flags != '':
            abi_link.add(self.stack_protector_flags)

        abi_flags = ' '.join(sorted(abi_link))

        return abi_flags

    def cc_warning_flags(self, options):
        def gen_flags():
            yield self.warning_flags

        return (' '.join(gen_flags())).strip()

    def cc_lang_flags(self):
        return self.lang_flags

    def cc_compile_flags(self, options, with_debug_info=None, enable_optimizations=None):
        def gen_flags(with_debug_info, enable_optimizations):

            sanitizers_enabled = False

            if with_debug_info is None:
                with_debug_info = options.with_debug_info
            if enable_optimizations is None:
                enable_optimizations = not options.no_optimizations

            if with_debug_info:
                yield self.debug_info_flags

            if enable_optimizations:
                if options.optimize_for_size:
                    if self.size_optimization_flags != '':
                        yield self.size_optimization_flags
                    else:
                        logging.warning("No size optimization flags set for current compiler")
                        yield self.optimization_flags
                elif sanitizers_enabled and self.sanitizer_optimization_flags != '':
                    yield self.sanitizer_optimization_flags
                else:
                    yield self.optimization_flags

            if options.arch in self.cpu_flags:
                yield self.cpu_flags[options.arch]

            if options.arch in self.cpu_flags_no_debug:

                # Only enable these if no debug/sanitizer options enabled

                if not (options.debug_mode or sanitizers_enabled):
                    yield self.cpu_flags_no_debug[options.arch]

            yield self.large_files

        return (' '.join(gen_flags(with_debug_info, enable_optimizations))).strip()

    @staticmethod
    def _so_link_search(osname, debug_info):
        so_link_typ = [osname, 'default']
        if debug_info:
            so_link_typ = [l + '-debug' for l in so_link_typ] + so_link_typ
        return so_link_typ

    def so_link_command_for(self, osname, options):
        """
        Return the command needed to link a shared object
        """

        for s in self._so_link_search(osname, options.with_debug_info):
            if s in self.so_link_commands:
                return self.so_link_commands[s]

        raise InternalError(
            "No shared library link command found for target '%s' in compiler settings '%s'" %
            (osname, self.infofile))

    def binary_link_command_for(self, osname, options):
        """
        Return the command needed to link an app/test object
        """

        for s in self._so_link_search(osname, options.with_debug_info):
            if s in self.binary_link_commands:
                return self.binary_link_commands[s]

        return '$(LINKER)'

class SourcePaths(object):
    """
    A collection of paths defined by the project structure and
    independent of user configurations.
    All paths are relative to the base_dir, which may be relative as well (e.g. ".")
    """

    def __init__(self, base_dir):
        self.base_dir = base_dir
        self.src_dir = os.path.join(self.base_dir, 'src')

        # dirs in src/
        self.build_data_dir = os.path.join(self.src_dir, 'build-data')

class BuildPaths(object): # pylint: disable=too-many-instance-attributes
    """
    Constructor
    """
    def __init__(self, source_paths, options, modules):
        self.build_dir = os.path.join(options.with_build_dir, 'build')

        self.libobj_dir = os.path.join(self.build_dir, 'obj', 'lib')
        self.depsobj_dir = os.path.join(self.build_dir, 'obj', 'deps')
        self.cliobj_dir = os.path.join(self.build_dir, 'obj', 'cli')
        self.testobj_dir = os.path.join(self.build_dir, 'obj', 'test')

        self.include_dir = os.path.join(self.build_dir, 'include')
        self.internal_include_dir = os.path.join(self.include_dir, 'internal')
        self.external_include_dir = os.path.join(self.include_dir, 'external')

        self.internal_headers = sorted(flatten([m.internal_headers() for m in modules]))
        self.external_headers = sorted(flatten([m.external_headers() for m in modules]))

        self.lib_sources = [normalize_source_path(s) for s in
                sorted(flatten([mod.sources() for mod in modules if not mod.third_party ]))]

        self.deps_sources = [normalize_source_path(s) for s in
                sorted(flatten([mod.sources() for mod in modules if mod.third_party ]))]

        def find_sources_in(basedir, srcdir):
            for (dirpath, _, filenames) in os.walk(os.path.join(basedir, srcdir)):
                for filename in filenames:
                    if filename.endswith('.cpp') and not filename.startswith('.'):
                        yield os.path.join(dirpath, filename)

        def find_headers_in(basedir, srcdir):
            for (dirpath, _, filenames) in os.walk(os.path.join(basedir, srcdir)):
                for filename in filenames:
                    if filename.endswith('.h') and not filename.startswith('.'):
                        yield os.path.join(dirpath, filename)

        self.cli_sources = [normalize_source_path(s) for s in find_sources_in(source_paths.src_dir, 'cli')]
        self.cli_headers = [normalize_source_path(s) for s in find_headers_in(source_paths.src_dir, 'cli')]

        self.test_sources = [normalize_source_path(s) for s in find_sources_in(source_paths.src_dir, 'test')]
        self.test_headers = [normalize_source_path(s) for s in find_headers_in(source_paths.src_dir, 'test')]

        self.public_headers = sorted(flatten([m.public_headers() for m in modules]))

    def build_dirs(self):
        out = [
            self.libobj_dir,
            self.depsobj_dir,
            self.cliobj_dir,
            self.testobj_dir,
            self.internal_include_dir,
            self.external_include_dir,
        ]
        return out

    def format_include_paths(self, cc, external_include):
        dash_i = cc.add_include_dir_option
        output = dash_i + self.include_dir
        if self.external_headers:
            output += ' ' + dash_i + self.external_include_dir
        if external_include:
            output += ' ' + dash_i + external_include
        return output

    def src_info(self, typ):
        if typ == 'lib':
            return (self.lib_sources, self.libobj_dir)
        if typ == 'deps':
            return (self.deps_sources, self.depsobj_dir)
        elif typ == 'cli':
            return (self.cli_sources, self.cliobj_dir)
        elif typ == 'test':
            return (self.test_sources, self.testobj_dir)

def create_template_vars(source_paths, build_paths, options, modules, cc, arch, osinfo):
    #pylint: disable=too-many-locals,too-many-branches,too-many-statements

    """
    Create the template variables needed to process the makefile, build.h, etc
    """

    def external_link_cmd():
        return (' ' + cc.add_lib_dir_option + options.with_external_libdir) if options.with_external_libdir else ''

    def link_to(module_member_name):
        """
        Figure out what external libraries/frameworks are needed based on selected modules
        """
        if not (module_member_name == 'libs' or module_member_name == 'frameworks'):
            raise InternalError("Invalid argument")

        libs = set()
        for module in modules:
            for (osname, module_link_to) in getattr(module, module_member_name).items():
                if osname == 'all' or osname == osinfo.basename:
                    libs |= set(module_link_to)
                else:
                    match = re.match('^all!(.*)', osname)
                    if match is not None:
                        exceptions = match.group(1).split(',')
                        if osinfo.basename not in exceptions:
                            libs |= set(module_link_to)

        return sorted(libs)

    def choose_mp_bits():
        mp_bits = arch.wordsize # allow command line override?
        logging.debug('Using MP bits %d' % (mp_bits))
        return mp_bits

    def configure_command_line():
        # Cut absolute path from main executable (e.g. configure.py or python interpreter)
        # to get the same result when configuring the same thing on different machines
        main_executable = os.path.basename(sys.argv[0])
        return ' '.join([main_executable] + sys.argv[1:])

    def cmake_escape(s):
        return s.replace('(', '\\(').replace(')', '\\)')

    def sysroot_option():
        if options.with_sysroot_dir == '':
            return ''
        if cc.add_sysroot_option == '':
            logging.error("This compiler doesn't support --sysroot option")
        return cc.add_sysroot_option + options.with_sysroot_dir

    def ar_command():
        if options.ar_command:
            return options.ar_command

        if cc.ar_command:
            if cc.ar_command == cc.binary_name:
                return options.compiler_binary or cc.binary_name
            else:
                return cc.ar_command

        return osinfo.ar_command

    def choose_endian(arch_info, options):
        if options.with_endian != None:
            return options.with_endian

        if options.cpu.endswith('eb') or options.cpu.endswith('be'):
            return 'big'
        elif options.cpu.endswith('el') or options.cpu.endswith('le'):
            return 'little'

        logging.info('Defaulting to assuming %s endian', arch_info.endian)
        return arch_info.endian

    build_dir = options.with_build_dir or os.path.curdir
    program_suffix = options.program_suffix or osinfo.program_suffix

    def join_with_build_dir(path):
        # For some unknown reason MinGW doesn't like ./foo
        if build_dir == os.path.curdir and options.os == 'mingw':
            return path
        return os.path.join(build_dir, path)

    variables = {
        'base_dir': source_paths.base_dir,
        'src_dir': source_paths.src_dir,
        'prefix': osinfo.lib_prefix,
        'libname': 'back_end',
        'command_line': configure_command_line(),
        'local_config': read_textfile(options.local_config),
        'includedir': options.includedir or osinfo.header_dir,

        'out_dir': build_dir,
        'build_dir': build_paths.build_dir,

        'makefile_path': os.path.join(build_paths.build_dir, '..', 'Makefile'),

        'libobj_dir': build_paths.libobj_dir,
        'depsobj_dir': build_paths.depsobj_dir,
        'cliobj_dir': build_paths.cliobj_dir,
        'testobj_dir': build_paths.testobj_dir,
        'os': options.os,
        'arch': options.arch,
        'cpu_family': arch.family,
        'endian': choose_endian(arch, options),
        'cpu_is_64bit': arch.wordsize == 64,
        'mp_bits': choose_mp_bits(),
        'python_exe': os.path.basename(sys.executable),
        'python_version': options.python_version,
        'cxx': (options.compiler_binary or cc.binary_name),
        'cxx_abi_flags': cc.mach_abi_link_flags(options),
        'linker': cc.linker_name or '$(CXX)',
        'make_supports_phony': cc.basename != 'msvc',
        'cc_compile_opt_flags': cc.cc_compile_flags(options, False, True),
        'cc_compile_debug_flags': cc.cc_compile_flags(options, True, False),

        # These are for CMake
        'cxx_abi_opt_flags': cc.mach_abi_link_flags(options, False),
        'cxx_abi_debug_flags': cc.mach_abi_link_flags(options, True),

        'dash_o': cc.output_to_object,
        'dash_c': cc.compile_flags,

        'cc_lang_flags': cc.cc_lang_flags(),
        'cc_sysroot': sysroot_option(),
        'cc_compile_flags': options.cxxflags or cc.cc_compile_flags(options),
        'ldflags': ' '.join([cc.gen_ldflags(options), options.ldflags if options.ldflags else '']),
        'cc_warning_flags': cc.cc_warning_flags(options),
        'output_to_exe': cc.output_to_exe,
        'cc_macro': cc.macro_name,

        'shared_flags': cc.gen_shared_flags(options),
        'visibility_attribute': cc.gen_visibility_attribute(options),

        'link_cmd': cc.so_link_command_for(osinfo.basename, options) + external_link_cmd(),
        'exe_link_cmd': cc.binary_link_command_for(osinfo.basename, options) + external_link_cmd(),
        'post_link_cmd': '',

        'ar_command': ar_command(),
        'ar_options': cc.ar_options or osinfo.ar_options,
        'ar_output_to': cc.ar_output_to,

        'link_to': ' '.join(
            [cc.add_lib_option + lib for lib in link_to('libs')] +
            [cc.add_framework_option + fw for fw in link_to('frameworks')]
        ),

        'include_paths': build_paths.format_include_paths(cc, options.with_external_includedir),
        'module_defines': sorted(flatten([m.defines() for m in modules])),

        'os_features': osinfo.enabled_features(options),
        'os_name': osinfo.basename,
        'cpu_features': arch.supported_isa_extensions(cc, options),

        'with_debug_asserts': options.with_debug_asserts,

        'mod_list': sorted([m.basename for m in modules])
        }

    # The name is always set because Windows build needs it
    variables['static_lib_name'] = 'ororo'

    return variables

def do_io_for_build(cc, arch, osinfo, using_mods, build_paths, source_paths, template_vars, options):
    try:
        robust_rmtree(build_paths.build_dir)
    except OSError as e:
        if e.errno != errno.ENOENT:
            logging.error('Problem while removing build dir: %s' % (e))

    for build_dir in build_paths.build_dirs():
        try:
            robust_makedirs(build_dir)
        except OSError as e:
            if e.errno != errno.EEXIST:
                logging.error('Error while creating "%s": %s' % (build_dir, e))

    def write_template(sink, template):
        with open(sink, 'w') as f:
            f.write(process_template(template, template_vars))

    def in_build_dir(p):
        return os.path.join(build_paths.build_dir, p)

    def in_build_data(p):
        return os.path.join(source_paths.build_data_dir, p)

    link_method = choose_link_method(options)

    def link_headers(headers, visibility, directory):
        logging.debug('Linking %d %s header files in %s' % (len(headers), visibility, directory))

        for header_file in headers:
            try:
                portable_symlink(header_file, directory, link_method)
            except OSError as e:
                if e.errno != errno.EEXIST:
                    raise UserError('Error linking %s into %s: %s' % (header_file, directory, e))

    link_headers(build_paths.public_headers, 'public',
                 build_paths.include_dir)

    link_headers(build_paths.internal_headers, 'internal',
                 build_paths.internal_include_dir)

    link_headers(build_paths.external_headers, 'external',
                 build_paths.external_include_dir)

    template_vars.update(generate_build_info(build_paths, using_mods, cc, arch, osinfo, options))
    with open(os.path.join(build_paths.build_dir, 'build_config.json'), 'w') as f:
        json.dump(template_vars, f, sort_keys=True, indent=2)
    makefile_template = os.path.join(source_paths.build_data_dir, 'makefile.in')
    write_template(template_vars['makefile_path'], makefile_template)

    if not options.without_qt_ui:
        build_dir = os.path.join('build','qt_build')
        if not os.path.isdir(build_dir):
            os.mkdir(build_dir)
        qt_pro_path = os.path.join('build','qt_build','EncryptPad.pro')
        qt_pro_template = os.path.join('qt_ui', 'EncryptPad.pro.in')
        write_template(qt_pro_path, qt_pro_template)

def yield_objectfile_list(sources, obj_dir, obj_suffix):
    obj_suffix = '.' + obj_suffix

    for src in sources:
        (directory, filename) = os.path.split(os.path.normpath(src))
        parts = directory.split(os.sep)

        if 'deps' in parts:
            parts = parts[parts.index('deps')+1:]
        elif 'src' in parts:
            parts = parts[parts.index('src')+1:]
        else:
            raise InternalError("Unexpected file '%s/%s'" % (directory, filename))

        if parts != []:
            # Handle src/X/X.cpp -> X.o
            if filename == parts[-1] + '.cpp':
                name = '_'.join(parts) + '.cpp'
            else:
                name = '_'.join(parts) + '_' + filename

            def fixup_obj_name(name):
                def remove_dups(parts):
                    last = None
                    for part in parts:
                        if last is None or part != last:
                            last = part
                            yield part

                return '_'.join(remove_dups(name.split('_')))

            name = fixup_obj_name(name)
        else:
            name = filename

        name = name.replace('.cpp', obj_suffix)
        yield os.path.join(obj_dir, name)

# Mutates `options`
def canonicalize_options(options, info_os, info_arch):
    # pylint: disable=too-many-branches
    if options.os not in info_os:
        def find_canonical_os_name(os_name_variant):
            for (canonical_os_name, os_info) in info_os.items():
                if os_info.matches_name(os_name_variant):
                    return canonical_os_name
            return os_name_variant # not found
        options.os = find_canonical_os_name(options.os)

    # canonical ARCH/CPU
    options.arch = canon_processor(info_arch, options.cpu)
    if options.arch is None:
        raise UserError('Unknown or unidentifiable processor "%s"' % (options.cpu))

    if options.cpu != options.arch:
        logging.info('Canonicalized CPU target %s to %s', options.cpu, options.arch)

    if options.with_stack_protector is None:
        if options.os in info_os:
            options.with_stack_protector = info_os[options.os].use_stack_protector

def generate_build_info(build_paths, modules, cc, arch, osinfo, options):
    # pylint: disable=too-many-locals

    # first create a map of src_file->owning module

    module_that_owns = {}

    for mod in modules:
        for src in mod.sources():
            module_that_owns[src] = mod

    def _isa_specific_flags(src):

        if src in module_that_owns:
            module = module_that_owns[src]
            isas = module.need_isa

            return cc.get_isa_specific_flags(isas, arch, options)

        return ''

    def _build_info(sources, objects, target_type):
        output = []
        for (obj_file, src) in zip(objects, sources):
            info = {
                'src': src,
                'obj': obj_file,
                'isa_flags': _isa_specific_flags(src)
                }

            output.append(info)

        return output

    out = {}

    targets = ['lib','deps','cli','test']

    out['isa_build_info'] = []

    fuzzer_bin = []
    for t in targets:
        src_list, src_dir = build_paths.src_info(t)

        src_key = '%s_srcs' % (t)
        obj_key = '%s_objs' % (t)
        build_key = '%s_build_info' % (t)

        objects = []
        build_info = []

        if src_list is not None:
            src_list.sort()
            objects = list(yield_objectfile_list(src_list, src_dir, osinfo.obj_suffix))
            build_info = _build_info(src_list, objects, t)

            for b in build_info:
                if b['isa_flags'] != '':
                    out['isa_build_info'].append(b)

            if t == 'fuzzer':
                fuzzer_bin = [b['exe'] for b in build_info]

        out[src_key] = src_list if src_list else []
        out[obj_key] = objects
        out[build_key] = build_info

    return out

def probe_qmake(options):
    qmake_found = False
    if have_program(options.qmake_bin):
        qmake_found = True

    if not qmake_found and have_program("qmake-qt5"):
        options.qmake_bin = "qmake-qt5"
        qmake_found = True

def execute_qmake(options):
    """
    run qmake to generate its Makefile
    """
    build_dir = os.path.join('build','qt_build')
    # if not os.path.isdir(build_dir):
    #     os.mkdir(build_dir)

    configs = []
    configs.append('debug' if options.debug_mode else 'release')
    spec_table = {
            'linux':'linux-g++',
            'freebsd':'freebsd-clang',
            'openbsd':'freebsd-clang',
            'darwin':'macx-clang',
            'windows':'win32-g++',
            'mingw':'win32-g++',
            }

    cmd = [options.qmake_bin, '-r']
    spec_name = spec_table.get(options.os)
    if spec_name:
        cmd.append('-spec')
        cmd.append(spec_name)
    cmd.extend(['CONFIG+=%s' % (' '.join(configs)), 'EncryptPad.pro'])

    logging.info('Executing: %s in %s', ' '.join(cmd), build_dir)
    try:
        qmake_proc = subprocess.Popen(cmd, cwd = build_dir)
        result = qmake_proc.wait()
    except OSError as e:
        raise UserError('Error while executing qmake: %s' % e)

    if result != 0:
        raise UserError('%s returned non zero exit code' % options.qmake_bin)

def get_project_dir():
    return os.path.abspath(os.curdir)

def get_botan_build_dir():
    return os.path.join(get_project_dir(), 'deps', 'botan')

def configure_botan(options):
    botan_dir = get_botan_build_dir()

    # There is an option in botan to change the build directory
    # Unfortunately it does not work with amalgamation
    botan_build_dir = botan_dir

    if not os.path.isdir(botan_build_dir):
        os.mkdir(botan_build_dir)

    cmd = [
            sys.executable,
            './configure.py',
            # '--with-build-dir', botan_build_dir,
            '--cc', options.compiler,
            '--cpu', options.cpu,
            '--os', options.os,
            ]

    if options.link_method:
        cmd.extend(['--link-method', options.link_method])
    if options.cxxflags:
        cmd.extend(['--cxxflags', options.cxxflags])
    if options.ldflags:
        cmd.extend(['--ldflags', options.ldflags])

    cmd.extend(['--amalgamation',
            '--disable-shared',
            '--with-zlib',
            '--with-bzip2',
            '--enable-modules', 'aes,pbkdf2,auto_rng,compression'])

    extra_flags = ""
    if options.build_zlib:
        extra_flags += ' ' if extra_flags else ''
        extra_flags += '-I ' + get_zlib_dir()
    if options.build_bzip2:
        extra_flags += ' ' if extra_flags else ''
        extra_flags += '-I ' + get_bzip2_dir()

    if extra_flags:
        cmd.extend(['--extra-cxxflags', extra_flags])

    logging.info('Executing: %s', ' '.join(cmd))

    try:
        botan_proc = subprocess.Popen(cmd, cwd = botan_dir)
        result = botan_proc.wait()
    except OSError as e:
        raise UserError('Error while executing commands: %s' % e)

def get_zlib_dir():
    return os.path.join(get_project_dir(), 'deps', 'zlib')

def get_bzip2_dir():
    return os.path.join(get_project_dir(), 'deps', 'bzip2')

def is_windows(options):
    return options.os in ['windows','mingw']

def configure_zlib(options):
    if is_windows(options):
        return

    zlib_dir = get_zlib_dir()
    cmd = [
            './configure',
            '--static']
    logging.info('Executing: %s', ' '.join(cmd))
    try:
        zlib_proc = subprocess.Popen(cmd, cwd = zlib_dir)
        result = zlib_proc.wait()
    except OSError as e:
        raise UserError('Error while executing commands: %s' % e)

def get_encryptmsg_dir():
    return os.path.join(get_project_dir(), 'deps', 'libencryptmsg')

def configure_encryptmsg(options):
    dir = get_encryptmsg_dir()
    cmd = [
            sys.executable,
            './configure.py',
            '--cc', options.compiler,
            '--cpu', options.cpu,
            '--os', options.os,
            '--static-linking',
            '--deps-dir', os.path.join(get_project_dir(), 'deps'),
            ]

    if options.debug_mode:
        cmd.extend(['--debug-mode'])
    if options.link_method:
        cmd.extend(['--link-method', options.link_method])
    if options.cxxflags:
        cmd.extend(['--cxxflags', options.cxxflags])
    if options.ldflags:
        cmd.extend(['--ldflags', options.ldflags])
    if options.build_botan:
        cmd.extend([
            '--botan-include-dir', get_botan_include_dir(),
            '--botan-lib-dir', get_botan_build_dir(),
            ])
    if options.build_zlib:
        cmd.extend([
            '--zlib-lib-dir', get_zlib_dir(),
            ])
    if options.build_bzip2:
        cmd.extend([
            '--bzip2-lib-dir', get_bzip2_dir(),
            ])
    logging.info('Executing: %s', ' '.join(cmd))
    try:
        encryptmsg_proc = subprocess.Popen(cmd, cwd = dir)
        result = encryptmsg_proc.wait()
    except OSError as e:
        raise UserError('Error while executing commands: %s' % e)

class ExternalCommandResult:
    def __init__(self, success, result):
        self.success = success
        self.result = result

def try_external_command(cmd):
    logging.info('Executing: %s', ' '.join(cmd))
    result = ''
    try:
        proc = subprocess.Popen(cmd, stdout=subprocess.PIPE)
        result, errs = proc.communicate()
    except OSError as e:
        raise UserError('Error while executing command: %s' % e)
    success = proc.returncode == 0
    result = result if result is str else result.decode("ascii")
    r = ExternalCommandResult(success, result)
    return r

def external_command(cmd):
    logging.info('Executing: %s', ' '.join(cmd))
    result = ''
    try:
        proc = subprocess.Popen(cmd, stdout=subprocess.PIPE)
        result, errs = proc.communicate()
    except OSError as e:
        raise UserError('Error while executing command: %s' % e)
    return result if result is str else result.decode("ascii")

def set_bzip2_variables(options, template_vars, cc):
    template_vars['build_bzip2'] = options.build_bzip2
    if options.build_bzip2:
        bzip2_dir = get_bzip2_dir()
        template_vars['bzip2_dir'] = bzip2_dir
        template_vars['bzip2_target'] = os.path.join(bzip2_dir, 'libbz2.a')
        template_vars['bzip2_make_target'] = 'libbz2.a'

        template_vars['bzip2_cxxflags'] =  cc.add_include_dir_option + bzip2_dir
        template_vars['bzip2_ldflags'] = os.path.join(bzip2_dir, 'libbz2.a')
    else:
        if try_external_command(['pkg-config','--exists','bzip2']).success:
            template_vars['bzip2_cxxflags'] = external_command(['pkg-config', '--cflags', 'bzip2'])
            template_vars['bzip2_ldflags'] = external_command(['pkg-config', '--libs', 'bzip2'])
        else:
            template_vars['bzip2_ldflags'] = "-lbz2"


def set_zlib_variables(options, template_vars, cc):
    template_vars['build_zlib'] = options.build_zlib
    if options.build_zlib:
        zlib_dir = get_zlib_dir()
        template_vars['zlib_dir'] = zlib_dir
        template_vars['zlib_target'] = os.path.join(zlib_dir, 'libz.a')

        template_vars['zlib_cxxflags'] =  cc.add_include_dir_option + zlib_dir
        template_vars['zlib_ldflags'] = os.path.join(zlib_dir, 'libz.a')

        template_vars['zlib_distclean_target'] = 'clean' if is_windows(options) else 'distclean'

        if options.os == 'mingw':
            template_vars['zlib_makefile'] = '-f win32/Makefile.gcc'
        elif is_windows(options):
            template_vars['zlib_makefile'] = '-f win32/Makefile.msc'
        else:
            template_vars['zlib_makefile'] = ''

    else:
        template_vars['zlib_cxxflags'] = external_command(['pkg-config', '--cflags', 'zlib'])
        template_vars['zlib_ldflags'] = external_command(['pkg-config', '--libs', 'zlib'])

def get_botan_include_dir():
    return os.path.join(get_project_dir(), 'deps', 'botan', 'build', 'include')

def set_botan_variables(options, template_vars, cc):
    template_vars['build_botan'] = options.build_botan
    if not options.build_botan:
        botan_cxxflags = external_command(['pkg-config', '--cflags', 'botan-2'])
        botan_ldflags = external_command(['pkg-config', '--libs', 'botan-2'])

        # remove all existing flags to avoid repetition
        all_flags = []
        for name in ['cc_sysroot', 'cxx_abi_flags', 'ldflags']:
            flags = template_vars[name]
            all_flags.extend(flags.split())

        received_flags = botan_ldflags.split()
        botan_ldflags = ' '.join(flag for flag in received_flags if flag not in all_flags)

    else:
        botan_build_dir = get_botan_build_dir()
        botan_target = os.path.join(botan_build_dir, 'libbotan-2.a')
        template_vars['botan_target'] = botan_target
        template_vars['botan_build_dir'] = botan_build_dir
        botan_cxxflags = cc.add_include_dir_option + get_botan_include_dir()
        botan_ldflags = botan_target

    template_vars['botan_cxxflags'] = botan_cxxflags
    template_vars['botan_ldflags'] = botan_ldflags

def set_encryptmsg_variables(options, template_vars, cc):
    dir = os.path.join(get_project_dir(), 'deps', 'libencryptmsg')
    template_vars['encryptmsg_build_dir'] = dir
    template_vars['encryptmsg_target'] = os.path.join(dir, 'build', 'libencryptmsg.a')
    template_vars['encryptmsg_cxxflags'] = '-I' + os.path.join(dir, 'build', 'include')
    template_vars['encryptmsg_ldflags'] = os.path.join(dir, 'build', 'libencryptmsg.a')

def process_command_line(args):

    parser = optparse.OptionParser(
        formatter=optparse.IndentedHelpFormatter(max_help_position=50))

    build_group = optparse.OptionGroup(parser, 'Build options')

    # EncryptPad parameters
    build_group.add_option('--without-qt-ui', dest='without_qt_ui', action='store_true', default=False,
                           help='include the libraries and CLI targets without Qt UI')
    build_group.add_option('--test', action='store_true', default=False,
                           help='include the unit tests without other targets')

    # Generic parameters
    build_group.add_option('--debug-mode', action='store_true', default=False,
            help='debug configuration. If not specified, the release configuration is used.')

    link_methods = ['symlink', 'hardlink', 'copy']
    build_group.add_option('--link-method', default=None, metavar='METHOD',
                           choices=link_methods,
                           help='choose how links to include headers are created (%s)' % ', '.join(link_methods))

    build_group.add_option('--os', help='set the target operating system')
    build_group.add_option('--cc', dest='compiler', help='set the desired build compiler')
    build_group.add_option('--cc-bin', dest='compiler_binary', metavar='BINARY',
                            help='set path to compiler binary')
    build_group.add_option('--cpu', help='set the target CPU architecture')
    build_group.add_option('--with-build-dir', metavar='DIR', default='',
                           help='setup the build in DIR')
    build_group.add_option('--program-suffix', metavar='SUFFIX',
                             help='append string to program names')
    build_group.add_option('--with-local-config',
                           dest='local_config', metavar='FILE',
                           help='include the contents of FILE into build.h')
    build_group.add_option('--includedir', metavar='DIR',
                             help='set the include file install dir')
    build_group.add_option('--with-endian', metavar='ORDER', default=None,
                            help='override byte order guess')
    build_group.add_option('--with-python-versions', dest='python_version',
                           metavar='N.M',
                           default='%d.%d' % (sys.version_info[0], sys.version_info[1]),
                           help='where to install botan2.py (def %default)')
    build_group.add_option('--with-debug-info', action='store_true', default=False, dest='with_debug_info',
                           help='include debug symbols')
    build_group.add_option('--with-stack-protector', dest='with_stack_protector',
                           action='store_false', default=None, help=optparse.SUPPRESS_HELP)
    build_group.add_option('--optimize-for-size', dest='optimize_for_size',
                           action='store_true', default=False,
                           help='optimize for code size')
    build_group.add_option('--with-sysroot-dir', metavar='DIR', default='',
                           help='use DIR for system root while cross-compiling')

    build_group.add_option('--cxxflags', metavar='FLAG', default=None,
                            help='set compiler flags')
    build_group.add_option('--no-optimizations', dest='no_optimizations',
                           action='store_true', default=False,
                           help='disable all optimizations (for debugging)')
    build_group.add_option('--ldflags', metavar='FLAG',
                            help='set linker flags', default=None)
    build_group.add_option('--with-external-libdir', metavar='DIR', default='',
                           help='use DIR for external libs')
    build_group.add_option('--ar-command', dest='ar_command', metavar='AR', default=None,
                            help='set path to static archive creator')
    build_group.add_option('--with-external-includedir', metavar='DIR', default='',
                           help='use DIR for external includes')
    build_group.add_option('--without-os-features', action='append', metavar='FEAT',
                            help='specify OS features to disable')
    build_group.add_option('--with-debug-asserts', action='store_true', default=False,
                           help=optparse.SUPPRESS_HELP)

    build_group.add_option('--static', action='store_true', dest='static_linking', default=False,
                            help='link the application statically')

    build_group.add_option('--qmake-bin', dest='qmake_bin', metavar='QMAKE', default='qmake',
                            help='set path to qmake binary')

    build_group.add_option('--build-zlib', action='store_true', dest='build_zlib', default=False,
                            help='build zlib from source. If not set, use the system library')
    build_group.add_option('--build-bzip2', action='store_true', dest='build_bzip2', default=False,
                            help='build bzip2 from source. If not set, use the system library')
    build_group.add_option('--build-botan', action='store_true', dest='build_botan', default=False,
                            help='build botan from source. If not set, use the system library')

    parser.add_option_group(build_group)
    (options, args) = parser.parse_args(args)

    if args != []:
        raise UserError('Unhandled option(s): ' + ' '.join(args))

    if options.with_endian not in [None, 'little', 'big']:
        raise UserError('Bad value to --with-endian "%s"' % (options.with_endian))

    if options.debug_mode:
        options.no_optimizations = True
        options.with_debug_info = True

    options.enabled_modules = []
    options.disabled_modules = []
    options.with_os_features = []
    options.without_os_features = []
    options.disable_intrinsics = []

    return options

def probe_environment(options):
    if options.build_botan and options.build_zlib and options.build_bzip2:
        return

    if not have_program('pkg-config'):
        raise UserError('pkg-config is not found. Enable all --build-... options')

    if not options.build_botan and not try_external_command(['pkg-config','--exists','botan-2']).success:
        raise UserError('botan-2 package is not found. Use --build-botan')

    if not options.build_zlib and not try_external_command(['pkg-config','--exists','zlib']).success:
        raise UserError('bzip2 package is not found. Use --build-zlib')


def configure_back_end(system_command, options):
    """
    Build the back_end library and cli
    """

    source_paths = SourcePaths(os.path.dirname(system_command))
    info_arch = load_build_data_info_files(source_paths, 'CPU info', 'arch', ArchInfo)
    info_os = load_build_data_info_files(source_paths, 'OS info', 'os', OsInfo)
    info_cc = load_build_data_info_files(source_paths, 'compiler info', 'cc', CompilerInfo)

    set_defaults_for_unset_options(options, info_arch, info_cc)
    canonicalize_options(options, info_os, info_arch)
    probe_environment(options)

    logging.info('Autodetected platform information: OS="%s" machine="%s" proc="%s"',
                 platform.system(), platform.machine(), platform.processor())

    info_modules = load_info_files('src', 'Modules', 'info.txt', ModuleInfo)
    for mod in ['state_machine','botan_1_openpgp_codec','stlplus']:
        info_modules.update(
                load_info_files(os.path.join('deps', mod),
                'Modules', 'info.txt', ModuleInfo)
                )

    build_paths = BuildPaths(source_paths, options, info_modules.values())
    cc = info_cc[options.compiler]
    arch = info_arch[options.arch]
    osinfo = info_os[options.os]
    template_vars = create_template_vars(source_paths, build_paths, options, info_modules.values(),
            cc, arch, osinfo)
    template_vars['library_target'] = os.path.join(get_project_dir(), build_paths.build_dir, 'libback_end.a')
    target_dir = os.path.join(get_project_dir(), 'bin', 'debug' if options.debug_mode else 'release')
    if not os.path.isdir(target_dir):
        robust_makedirs(target_dir)
    template_vars['cli_exe'] = os.path.join(target_dir, 'encryptcli')
    template_vars['test_exe'] = os.path.join(target_dir, 'encryptpad_test')
    template_vars['deps_dir'] = os.path.join(get_project_dir(), 'deps')

    include_paths_items = [
            ('deps','libencryptmsg','build','include'),
            ('build','include','internal'),
            ]

    include_paths = template_vars['include_paths']
    for item in include_paths_items:
        include_paths += ' ' + cc.add_include_dir_option + os.path.join(get_project_dir(), *item)

    include_paths_items = [
            ('deps','stlplus','containers'),
            ('deps','plog','include'),
            ]
    for item in include_paths_items:
        include_paths += ' -isystem ' + os.path.join(get_project_dir(), *item)

    template_vars['include_paths'] = include_paths

    #qt build
    template_vars['build_qt_ui'] = not options.without_qt_ui
    template_vars['qt_build_dir'] = os.path.join('build', 'qt_build')
    template_vars['debug_mode'] = options.debug_mode
    default_targets = ['libs','cli']
    if not options.without_qt_ui:
        default_targets.append('qt_ui')
        if options.os == 'darwin':
            default_targets.append('macos_deployment')

    if options.test:
        default_targets.append('test')

    template_vars['default_targets'] = ' '.join(default_targets)

    set_botan_variables(options, template_vars, cc)
    set_zlib_variables(options, template_vars, cc)
    set_bzip2_variables(options, template_vars, cc)
    set_encryptmsg_variables(options, template_vars, cc)

    do_io_for_build(cc, arch, osinfo, info_modules.values(), build_paths, source_paths, template_vars, options)

def main(argv):
    """
    Main driver
    """

    # pylint: disable=too-many-locals
    options = process_command_line(argv[1:])
    setup_logging(options)
    logging.info('%s invoked with options "%s"', argv[0], ' '.join(argv[1:]))

    configure_back_end(argv[0], options)
    if not options.without_qt_ui:
        probe_qmake(options)
        execute_qmake(options)

    if options.build_botan:
        configure_botan(options)
    if options.build_zlib:
        configure_zlib(options)
    configure_encryptmsg(options)
    return 0

if __name__ == '__main__':
    try:
        sys.exit(main(argv=sys.argv))
    except UserError as e:
        logging.debug(traceback.format_exc())
        logging.error(e)
    except Exception as e: # pylint: disable=broad-except
        # error() will stop script, so wrap all information into one call
        logging.error("""%s
An internal error occurred.

Don't panic, this is probably not your fault!
""" % traceback.format_exc())

    sys.exit(0)
