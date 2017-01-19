#!/usr/bin/env python

# Generate in OpenSCAD a N x M maze
# The maze itself is generated using a depth first search (DFS)

# Written by Daniel C. Newman, 8 June 2012

# This code is adapted from eggbot_maze.py which was written by
# Daniel C. Newman (myself) with improvements and suggestions
# from W. Craig Trader, 20 September 2010

# Usage: maze.py [-cfhlrtw] [output-file]
#   -c, --columns
#      Number of maze columns running the maze width (default = 8)
#   -f, --floor
#      Physical thickness in millimeters of the maze floor (default = 2.000000)
#   -h, --height
#      Physical height in millimeters of the maze (default = 8.000000)
#   -l, --length
#      Physical length in millimeters of the maze (default = 30.000000)
#   -r, --rows
#      Number of maze rows running the maze height (default = 8)
#   -t, --thickness
#      Physical maze wall thickness in millimeters (default = 1.000000)
#   -w, --width`
#      Physical width in millimeters of the maze (default = 30.000000)
#   output-file
#      Name of the output file to generate; if not specified then
#      output will be written to standard output

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

import sys
import getopt
import array
import random
import math

DEFAULT_HEIGHT          =  5.0
DEFAULT_LENGTH          = 30.0
DEFAULT_WIDTH           = 30.0
DEFAULT_FLOOR_THICKNESS =  1.0
DEFAULT_WALL_THICKNESS  =  0.4

DEFAULT_ROWS            = 8
DEFAULT_COLS            = 8

# Initialize the psuedo random number generator
random.seed()

def usage( prog, exit_stat=0 ):
	a = float( DEFAULT_WIDTH ) / float( DEFAULT_COLS );
	b = float( DEFAULT_LENGTH ) / float( DEFAULT_ROWS );
	if b < a:
		a = b
	wt = 0.1 * a
	str  = 'Usage: %s [-cfhlrtw] [output-file]\n' % prog
	str += \
'  -c, --columns\n' + \
'     Number of maze columns running the maze width (default = %d)\n' % DEFAULT_COLS
	str += \
'  -f, --floor\n' + \
'     Physical thickness in millimeters of the maze floor (default = %f)\n' % DEFAULT_FLOOR_THICKNESS
	str += \
'  -h, --height\n' + \
'     Physical height in millimeters of the maze (default = %f)\n' % DEFAULT_HEIGHT
	str += \
'  -l, --length\n' + \
'     Physical length in millimeters of the maze (default = %f)\n' % DEFAULT_LENGTH
	str += \
'  -r, --rows\n' + \
'     Number of maze rows running the maze length (default = %d)\n' % DEFAULT_ROWS
	str += \
'  -t, --thickness\n' + \
'     Physical maze wall thickness in millimeters (default = %f)\n' % wt
	str += \
'  -w, --width\n' + \
'     Physical width in millimeters of the maze (default = %f)\n' % DEFAULT_WIDTH
	str += \
'  output-file\n' + \
'     Name of the output file to generated; if not specified then\n' + \
'     output will be written to standard output\n'

	if exit_stat != 0:
		sys.stderr.write( str )
	else:
		sys.stdout.write( str)
	sys.exit( exit_stat )

class Maze:

	# Each cell in the maze is represented using 9 bits:
	#
	#  Visited -- When set, indicates that this cell has been visited during
	#             construction of the maze
	#
	#  Border  -- Four bits indicating which if any of this cell's walls are
	#             part of the maze's boundary (i.e., are unremovable walls)
	#
	#  Walls   -- Four bits indicating which if any of this cell's walls are
	#             still standing
	#
	#  Visited     Border      Walls
	#        x    x x x x    x x x x
	#             W S E N    W S E N

	_VISITED = 0x0100
	_NORTH   = 0x0001
	_EAST    = 0x0002
	_SOUTH   = 0x0004
	_WEST    = 0x0008

	def __init__( self, r=DEFAULT_ROWS, c=DEFAULT_COLS, l=DEFAULT_LENGTH,
		      w=DEFAULT_WIDTH, h=DEFAULT_HEIGHT, f=DEFAULT_FLOOR_THICKNESS,
		      t=DEFAULT_WALL_THICKNESS, file=None):

		self.w          = int( c )  # Number of cells wide
		self.h          = int( r )  # Number of cells high
		self.solved     = int( 0 )
		self.start_x    = int( 0 )
		self.start_y    = int( 0 )
		self.finish_x   = int( 0 )
		self.finish_y   = int( 0 )
		self.solution_x = None
		self.solution_y = None
		self.cells      = None

		# Drawing information
		self.height          = float( h )
		self.width           = float( w )
		self.length          = float( l )
		self.floor_thickness = float( f )
		self.wall_thickness  = float( t )

		# Output file
		self.f = file
		if self.f is None:
			self.f = sys.stdout
		self.f1 = open( "walloutput.txt", 'w' )
		self.f1.write('%f %f %f\n' % (float(h), float(f), float(t)) )

		# The large mazes tend to hit the recursion limit
		limit = sys.getrecursionlimit()
		if limit < ( 4 + self.w * self.h ):
			sys.setrecursionlimit( 4 + self.w * self.h )

		maze_size       = self.w * self.h
		self.finish_x   = int( self.w - 1 )
		self.finish_y   = int( self.h - 1 )
		self.solution_x = array.array( 'i', range( 0, maze_size ) )
		self.solution_y = array.array( 'i', range( 0, maze_size ) )
		self.cells      = array.array( 'H', range( 0, maze_size ) )

		# Start a new maze
		self.solved   = 0
		self.start_x  = random.randint( 0, self.w - 1 )
		self.finish_x = random.randint( 0, self.w - 1 )

		# Initialize every cell with all four walls up

		for i in range( 0, maze_size ):
			self.cells[i] = Maze._NORTH | Maze._EAST | Maze._SOUTH | Maze._WEST

		# Now set our borders -- borders being walls which cannot be removed.

		z = ( self.h - 1 ) * self.w
		for x in range( 0, self.w ):
			self.cells[x    ] |= Maze._NORTH << 4
			self.cells[x + z] |= Maze._SOUTH << 4

		for y in range( 0, self.h ):
			self.cells[         0 + y * self.w] |= Maze._WEST << 4
			self.cells[self.w - 1 + y * self.w] |= Maze._EAST << 4

		# Build the maze
		self.handle_cell( 0, self.start_x, self.start_y )

		# Now that the maze has been built, remove the appropriate walls
		# associated with the start and finish points of the maze

		# Note: we have to remove these after building the maze.  If we
		# remove them first, then the lack of a border at the start (or
		# finish) cell will allow the handle_cell() routine to wander
		# outside of the maze.  I.e., handle_cell() doesn't do boundary
		# checking on the cell cell coordinates it generates.  Instead, it
		# relies upon the presence of borders to prevent it wandering
		# outside the confines of the maze.

		self.remove_border( self.start_x, self.start_y, Maze._NORTH )
		self.remove_wall( self.start_x, self.start_y, Maze._NORTH )

		self.remove_border( self.finish_x, self.finish_y, Maze._SOUTH )
		self.remove_wall( self.finish_x, self.finish_y, Maze._SOUTH )

		# Now draw the maze

		# Note that each cell in the maze is drawn 2 units wide by
		# 2 units high.  A width and height of 2 was chosen for
		# convenience and for allowing easy identification (as the integer 1)
		# of the centerline along which to draw solution paths if desired.

		# Draw the OpenSCAD program's preamble

		self.preamble()

		# To draw the walls, we start at the left-most column of cells, draw down drawing
		# the WEST and NORTH walls and then draw up drawing the EAST and SOUTH walls.
		# By drawing in this back and forth fashion, we minimize the effect of slippage.

		for x in range( 0, self.w ):
			self.draw_vertical( x )

		self.postamble()

		# Restore the recursion limit
		sys.setrecursionlimit( limit )

	# Mark the cell at (x, y) as "visited"
	def visit( self, x, y ):
		self.cells[y * self.w + x] |= Maze._VISITED

	# Return a non-zero value if the cell at (x, y) has been visited
	def is_visited( self, x, y ):
		if self.cells[y * self.w + x] & Maze._VISITED:
			return -1
		else:
			return 0

	# Return a non-zero value if the cell at (x, y) has a wall
	# in the direction d
	def is_wall( self, x, y, d ):
		if self.cells[y * self.w + x] & d:
			return -1
		else:
			return 0

	# Remove the wall in the direction d from the cell at (x, y)
	def remove_wall( self, x, y, d ):
		self.cells[y * self.w + x] &= ~d

	# Return a non-zero value if the cell at (x, y) has a border wall
	# in the direction d
	def is_border( self, x, y, d ):
		if self.cells[y * self.w + x] & ( d << 4 ):
			return -1
		else:
			return 0

	# Remove the border in the direction d from the cell at (x, y)
	def remove_border( self, x, y, d ):
		self.cells[y * self.w + x] &= ~( d << 4 )

	# This is the DFS algorithm which builds the maze.  We start at depth 0
	# at the starting cell (self.start_x, self.start_y).  We then walk to a
	# randomly selected neighboring cell which has not yet been visited (i.e.,
	# previously walked into).  Each step of the walk is a recursive descent
	# in depth.  The solution to the maze comes about when we walk into the
	# finish cell at (self.finish_x, self.finish_y).
	#
	# Each recursive descent finishes when the currently visited cell has no
	# unvisited neighboring cells.
	#
	# Since we don't revisit previously visited cells, each cell is visited
	# no more than once.  As it turns out, each cell is visited, but that's a
	# little harder to show.  Net, net, each cell is visited exactly once.

	def handle_cell( self, depth, x, y ):

		# Mark the current cell as visited
		self.visit( x, y )

		# Save this cell's location in our solution trail / backtrace
		if not self.solved:

			self.solution_x[depth] = x
			self.solution_y[depth] = y

			if ( x == self.finish_x ) and ( y == self.finish_y ):
				# Maze has been solved
				self.solved = depth

		# Shuffle the four compass directions: this is the primary source
		# of "randomness" in the generated maze.  We need to visit each
		# neighboring cell which has not yet been visited.  If we always
		# did that in the same order, then our mazes would look very regular.
		# So, we shuffle the list of directions we try in order to find an
		# unvisited neighbor.

		# HINT: TRY COMMENTING OUT THE shuffle() BELOW AND SEE FOR YOURSELF

		directions = [Maze._NORTH, Maze._SOUTH, Maze._EAST, Maze._WEST]
		random.shuffle( directions )

		# Now from the cell at (x, y), look to each of the four
		# directions for unvisited neighboring cells

		for i in range( 0, 4 ):

			# If there is a border in direction[i], then don't try
			# looking for a neighboring cell in that direction.  We
			# Use this check and borders to prevent generating invalid
			# cell coordinates.

			if self.is_border( x, y, directions[i] ):
				continue

			# Determine the cell coordinates of a neighboring cell
			# NOTE: we trust the use of maze borders to prevent us
			# from generating invalid cell coordinates

			if directions[i] == Maze._NORTH:
				nx = x
				ny = y - 1
				opposite_direction = Maze._SOUTH

			elif directions[i] == Maze._SOUTH:
				nx = x
				ny = y + 1
				opposite_direction = Maze._NORTH

			elif directions[i] == Maze._EAST:
				nx = x + 1
				ny = y
				opposite_direction = Maze._WEST

			else:
				nx = x - 1
				ny = y
				opposite_direction = Maze._EAST

			# Wrap in the horizontal dimension
			if nx < 0:
				nx += self.w
			elif nx >= self.w:
				nx -= self.w

			# See if this neighboring cell has been visited
			if self.is_visited( nx, ny ):
				# Neighbor has been visited already
				continue

			# The neighboring cell has not been visited: remove the wall in
			# the current cell leading to the neighbor.  And, from the
			# neighbor remove its wall leading to the current cell.

			self.remove_wall(  x,  y, directions[i] )
			self.remove_wall( nx, ny, opposite_direction )

			# Now recur by "moving" to this unvisited neighboring cell

			self.handle_cell( depth + 1, nx, ny )

	def draw_line( self, x1, y1, x2, y2 ):
		if x1 == x2:
			self.f.write( '     wall_vert(%d, %d, %d);\n' % (x1, y1, y2) )
		else:
			self.f.write( '     wall_horz(%d, %d, %d);\n' % (x2, x1, y1) )
		self.f1.write('%d %d %d %d\n' % (x1, y1, x2, y2) )

	def draw_wall( self, x, y, d ):

		if d == Maze._NORTH:
			self.draw_line( 2*(x+1), 2*y, 2*x, 2*y )
		elif d == Maze._WEST:
			self.draw_line( 2*x, 2*y, 2*x, 2*(y+1) )
		elif d == Maze._SOUTH:
			self.draw_line( 2*(x+1), 2*(y+1), 2*x, 2*(y+1) )
		else:  # Mase._EAST
			self.draw_line( 2*(x+1), 2*y, 2*(x+1), 2*(y+1) )

	# Draw the walls of the maze along the column of cells at horizonal positions

	def draw_vertical( self, x ):

		# Drawing moving downwards from north to south

		if self.is_wall( x, 0, Maze._NORTH ):
			self.draw_wall( x, 0, Maze._NORTH )

		for y in range( 0, self.h ):
			if self.is_wall( x, y, Maze._WEST ):
				self.draw_wall( x, y, Maze._WEST )
			if self.is_wall( x, y, Maze._SOUTH ):
				self.draw_wall( x, y, Maze._SOUTH )
			if ( x == ( self.w - 1 ) ) and self.is_wall( x, y, Maze._EAST ):
				self.draw_wall( x, y, Maze._EAST )

	# Produce the static portion of the OpenSCAD program

	def preamble( self ):
		self.f.write( '// %d x %d maze\n' % ( self.w, self.h ) )
		self.f.write( 'fudge  = 0.01;\n' )
		self.f.write( 'width  = ' + str( self.width ) + ';\n' )
		self.f.write( 'length = ' + str( self.length ) + ';\n' )
		self.f.write( 'height = ' + str( self.height ) + ';\n' )
		self.f.write( 'floor_thickness = ' + str( self.floor_thickness ) + ';\n' )
		self.f.write( 'wall_thickness  = ' + str( self.wall_thickness ) + ';\n' )
		self.f.write( 'wall_height     = height - floor_thickness;\n' )
		self.f.write( '\n' )
		self.f.write( 'module wall_vert(x, y1, y2)\n' )
		self.f.write( '{\n' )
		self.f.write( '     translate([x, y1, floor_thickness - fudge]) cube([wall_thickness, (y2 - y1) + wall_thickness, wall_height + fudge], center=false);\n' )
		self.f.write( '}\n' )
		self.f.write( '\n' )
		self.f.write( 'module wall_horz(x1, x2, y)\n' )
		self.f.write( '{\n' )
		self.f.write( '     translate([x1, y, floor_thickness - fudge]) cube([(x2 - x1) + wall_thickness, wall_thickness, wall_height + fudge], center=false);\n' )
		self.f.write( '}\n' )
		self.f.write( '\n' )
		self.f.write( 'translate([-width / 2, -length / 2, 0]) scale([width / (wall_thickness + 2 * %d), length / (wall_thickness + 2 * %d), 1]) union()\n' % ( self.w, self.h ) )
		self.f.write( '{\n' )
		self.f.write( '     cube([wall_thickness + 2 * %d, wall_thickness + 2 * %d, floor_thickness], center=false);\n' % ( self.w, self.h ) )

	def postamble( self ):
		self.f.write( '}\n')

# -----------------------
# Our 'main'

columns   = DEFAULT_COLS
floor     = DEFAULT_FLOOR_THICKNESS
height    = DEFAULT_HEIGHT
length    = DEFAULT_LENGTH
rows      = DEFAULT_ROWS
thickness = DEFAULT_WALL_THICKNESS
width     = DEFAULT_WIDTH

try:
	opts, args = getopt.getopt( sys.argv[1:], 'c:f:h:l:r:t:w:',
				    [ 'columns=', 'floor=', 'height=', 'help',
				      'length=', 'rows=', 'thickness=', 'width=' ] )
except:
	usage( sys.argv[0], 1 )

if len( args ) > 1:
	usage( sys.argv[0], 1 )
elif len( args ) == 1:
	f = open( args[0], 'w' )
else:
	f = sys.stdout

thickness_set = False

for opt, val in opts:
	if opt in ( '-c', '--columns' ):
		columns = int( val )
	elif opt in ( '-f', '--floor' ):
		floor = float( val )
	elif opt in ( '-h', '--height' ):
		height = float( val )
	elif opt in ( '-l', '--length' ):
		length = float( val )
	elif opt in ( '-r', '--rows' ):
		rows = int( val )
	elif opt in ( '-t', '--thickness' ):
		thickness_set = True
		thickness = float( val )
	elif opt in ( '-w', '--width' ):
		width = float( val )
	elif opt in ( '?', '--help' ):
		usage( sys.argv[0], 0 )
	else:
		usage( sys.argv[0], 1 )

a = float( width ) / float( columns );
b = float( length ) / float( rows );
if b < a:
	a = b

#if not thickness_set:
#	thickness = 0.2 * a
#elif thickness > ( 0.5 * a ):
#	sys.stderr.write( 'Wall thickness exceeds twice the minimum cell ' + \
#			  'dimension;\nconsider using a smaller wall thickness\n')

Maze( rows, columns, length, width, height, floor, thickness, f )
