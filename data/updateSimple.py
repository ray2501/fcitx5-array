#!/usr/bin/python3
# -*- coding: utf-8 -*-

from sqlite3 import dbapi2 as sqlite
from sys import argv

def array_updatedb(table_file, table):
	con = sqlite.connect("array.db")
	cur = con.cursor()
	cur.execute('select * from ' + table)
	tbl = cur.fetchall()

	# read from the text table
	f = open(table_file, 'r')
	z = map(lambda x:x.split('\t'), filter(lambda k:(k[0] != '#' and k[0] != '%' and len(k.strip()) != 0), f.readlines()))
	k = map(lambda y:(y[0].lower(), y[1].strip(' \n')), z)
	f.close()

	# update the database
	for i, j in k:
		cur.execute('INSERT INTO ' + table + ' (keys, ch) VALUES ("' + i + '", "' + j + '");')

	con.commit()
	con.close()

# empty tables
con = sqlite.connect("array.db")
cur = con.cursor()
cur.execute('DELETE FROM simple;')
con.commit()
con.close()

array_updatedb(argv[1], 'simple')
