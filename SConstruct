# -*- python -*-
import os

DEFAULT_DEBUG = False

try:
    Import('env')
    imported_env = True
except:
    env = Environment()
    imported_env = False

    vars = Variables('build-setup.conf', ARGUMENTS)
    vars.Update(env)
    vars.Add('debug', '', DEFAULT_DEBUG)
    vars.Update(env)
    vars.Save('build-setup.conf', env)
    env['debug'] = (env['debug'] == True)

if not 'XA_INSTALL_DIR' in env:
    env['XA_INSTALL_DIR'] = os.path.join(Dir('#').abspath, 'BUILD/INSTALL')
if not 'XA_PREFIX' in env:
    env['XA_PREFIX'] = '/opt/xabyss/pca'

cppdefines = []
ccflags = ['-Wall']
linkflags = []

debug = env['debug']
if debug:
    cppdefines += ['-DDEBUG']
    ccflags += ['-g']
else:
    cppdefines += ['-DNDEBUG']

cxxflags = ['-std=c++11']

env.Append(CPPDEFINES = cppdefines)
env.Append(CCFLAGS = ccflags)
env.Append(CXXFLAGS = cxxflags)
env.Append(LINKFLAGS = linkflags)

Export('env')
SConscript(['doc/SConscript'])
SConscript(['src/SConscript'], exports=['env'])
