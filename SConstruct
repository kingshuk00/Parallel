###############################################################################
#   This SConstruct is to build MPI related utility class, Parallel
#   Copyright (C) 2018  Kingshuk
#   This program is free software: you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation, either version 3 of the License, or
#   (at your option) any later version.
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#   You should have received a copy of the GNU General Public License
#   along with this program.  If not, see <https://www.gnu.org/licenses/>.
###############################################################################

#source: <https://scons.org/doc/1.0.1/HTML/scons-user/x2361.html>

import os

env= Environment()

debug= ARGUMENTS.get("debug", 0)

env['CPPFLAGS']= ['-Wall', '-Werror']

if debug== 0:
    env.Append(CPPFLAGS= '-O3')
else:
    env.Append(CPPFLAGS= '-g3')

env['LIBPATH']= '.'
env['CPPPATH']= 'src'

#For MPI
homedir= os.environ['HOME']
env.Append(LIBPATH= [os.path.join(homedir, 'usr/lib')])
env.Append(CPPPATH= [os.path.join(homedir, 'usr/include')])

env.VariantDir('build', 'src', duplicate=0)

lib= env.SharedLibrary(target='build/parallel',
                       source=['build/Parallel.cpp'],
                       LIBS=['pthread', 'mpi']
)

env.Install(target="install/lib", source=lib)
env.Install(target="install/include", source=['build/Parallel.hpp'])
