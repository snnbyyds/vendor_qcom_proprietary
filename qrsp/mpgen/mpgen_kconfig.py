"""
Copyright (c) 2017 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
"""

import sys
import os
import re


class parameter:
	def __init__(self, name, value):
		self.name = name
		self.value = value
		return

class Kconfig(list):

	def __init__(self, config_text):
		'''
			Parses kernel .config provided as text
		'''
		for line in config_text.splitlines():
			t = line.split('=')
			if len(t) < 2 : continue
			p = parameter(t[0].strip(), t[1].strip())
			if p is not None:
				self.append(p)
		return

	def get_parameter(self, name):
		p = next((p for p in self if p.name == name), None)
		if (p is None) : print("kconfig.get_parameter({0}) --> None".format(name))
		else :
			print("kconfig.get_parameter({0}) --> {1}".format(p.name, p.value))
		return p

	def is_parameter(self, name, value):
		p = next((p for p in self if p.name == name), None)
		if (p is None) :
			print("kconfig.is_parameter({0}) --> None".format(name))
			return 1==0
		else :
			print("kconfig.is_parameter({0}) --> {1}".format(p.name, p.value))
			if (p.value == value) :
				return 1 == 1
			else :
				return 1 == 0


	def get_parameters_regex(self, regex):
		"""
		Yelds generator of parameters that match to given regex expression
		"""
		return (p for p in self if re.match(regex, p.name))

