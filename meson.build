###############################################################################
# NAME:             meson.build
#
# AUTHOR:           Ethan D. Twardy <ethan.twardy@gmail.com>
#
# DESCRIPTION:      Build script for the application
#
# CREATED:          11/04/2021
#
# LAST EDITED:      11/04/2021
#
# Copyright 2021, Ethan D. Twardy
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
###

project('git-touch', 'c')
libbsd = dependency('libbsd')
executable(
  'git-touch',
  'git-touch.c',
  c_args: ['-Wall', '-Wextra', '-Werror', '-O3'],
  install: true,
  dependencies: [libbsd],
)

install_data('LICENSE', install_dir: 'usr/share/licenses/git-touch')

###############################################################################
