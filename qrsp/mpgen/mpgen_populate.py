"""
Copyright (c) 2017-2018, 2020, 2022 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
"""
from __future__ import absolute_import
from __future__ import print_function

# Populates mpdb
# Assumes catalog is loaded
# Assumes kernel is loaded

import sys
import mpgen_kernel
import mpgen_mpdb
from mpgen_mpdb import MPDB

# Populating the read-only sections could be part of the catalog, but
# in the case where new sections are added extracting RO sections from the
# compiled kernel ensures any new sections are included in the MP's.
def populate_ro_sections(mpdb, kernel):
	a = iter(kernel.section_tbl)
	for section in a:
		if (('READONLY' in section.flags)
				and ('debug' not in section.name)
				and ('+' not in section.name)) :
			section.dump()
			sha256 = kernel.get_section_sha256(section.name)
			if ('CODE' in section.flags) :
				mpdb.append(mpgen_mpdb.MP_WR_ONCE_UNKNOWN(section.name,
														  section.VMA,
														  section.size,
														  '',
														  0,
														  sha256))
			else :
				mpdb.append(mpgen_mpdb.MP_RD_ONLY(section.name,
												  section.VMA,
												  section.size,
												  0,
												  sha256))

	return

def CalculateAddressAndSize(headTextAddr, symAddr, symOffset, symSize):
	newSymAddr = symAddr
	if headTextAddr != None and symAddr != None and symOffset != None:
		headTextAddr = int(headTextAddr, 16)
		symAddr = int(symAddr, 16)
		symOffset = int(symOffset, 16)
		symSize = int(symSize, 16)
		newSymAddr = headTextAddr + symOffset
		symSize = symSize - (newSymAddr - symAddr)
		newSymAddr = str(hex(newSymAddr)).strip("L")
		symSize = str(hex(symSize)).strip("L")
	return newSymAddr, symSize

def get_mpdb(catalog, kernel, config):
	mpdb = MPDB(catalog)
	a = iter(catalog)
	headTextAddr = None
	for x in a:
		try:
			# Authorized writers is a common accross attestation features
			writers = kernel.superset(x.writer)
			if (not writers) and (x.writer): catalog.mp_warning("`%s`(no %s)" % (x.name, x.writer))

			# x.dump()
			if x.mp_type is mpgen_mpdb.RD_ONLY :
				if x.section_or_symbol is mpgen_mpdb.SYMBOL :
					symbol = kernel.get_symbol(x.name)
					if (symbol is None) : continue
					if (symbol.section == "*ABS*"):
						print("Symbol %s is absolute value" % symbol.name)
						sha256 = "0"
					else:
						sha256 = kernel.get_symbal_sha256(x.name)
					mpdb.append(mpgen_mpdb.MP_RD_ONLY(symbol.name,
													  symbol.value,
													  symbol.size,
													  x.attrs,
													  sha256))
				else :
					if (x.name == 'populate_all_ro_sections') :
						populate_ro_sections(mpdb, kernel)
					else :
						section = kernel.get_section(x.name)
						if (('READONLY' in section.flags) and ('debug' not in section.name)) :
							section.dump()
							sha256 = kernel.get_section_sha256(section.name)
							mpdb.append(mpgen_mpdb.MP_RD_ONLY(section.name,
															  section.VMA,
															  section.size,
															  x.attrs,
															  sha256))
			if x.mp_type is mpgen_mpdb.WR_ONCE_KNOWN :
				symbol = kernel.get_symbol(x.name)
				if (symbol is None) : continue
				sha256 = kernel.get_symbal_sha256(x.name, x.value)
				mpdb.append(mpgen_mpdb.MP_WR_ONCE_KNOWN(
												symbol.name,
												symbol.value,
												symbol.size,
												writers,
												x.attrs,
												sha256))

			if x.mp_type is mpgen_mpdb.RD_ONLY_AFTER_INIT :
				symbol = kernel.get_symbol(x.name)
				section = kernel.get_section(x.name)
				if (symbol is None) : continue
				if (section is not None) : symbol.value, symbol.size = CalculateAddressAndSize(headTextAddr, symbol.value, section.file_off, symbol.size)
				sha256 = kernel.get_symbal_sha256(x.name)
				mpdb.append(mpgen_mpdb.MP_RD_ONLY_AFTER_INIT(
												symbol.name,
												symbol.value,
												symbol.size,
												writers,
												x.attrs,
												sha256))

			if x.mp_type is mpgen_mpdb.RD_WR :
				symbol = kernel.get_symbol(x.name)
				if (symbol is None) : continue
				symbol.value, symbol.size = CalculateAddressAndSize(headTextAddr, symbol.value, kernel.get_section(x.name).file_off, symbol.size)
				sha256 = kernel.get_symbal_sha256(x.name)
				mpdb.append(mpgen_mpdb.MP_RD_WR(
												symbol.name,
												symbol.value,
												symbol.size,
												writers,
												x.attrs,
												sha256))

			if x.mp_type is mpgen_mpdb.WR_ONCE_UNKNOWN :
				symbol = kernel.get_symbol(x.name)
				if(".head.text" == x.name) :
					headTextAddr = symbol.value
				if (symbol is None) : continue
				sha256 = kernel.get_symbal_sha256(x.name)
				mpdb.append(mpgen_mpdb.MP_WR_ONCE_UNKNOWN(
												symbol.name,
												symbol.value,
												symbol.size,
												writers,
												x.attrs,
												sha256))

			if x.mp_type is mpgen_mpdb.WR_ONCE_UNKNOWN_DWARF :
				symbol = kernel.get_symbol(x.name)
				if symbol is None : continue
				if not len(x.wu_ranges): raise mpgen_kernel.ExceptionDwarfParsingFailed(symbol.name)
				if x.wu_ranges[-1][1] > symbol.size: raise mpgen_kernel.ExceptionDwarfParsingFailed('chk.' + symbol.name)  # just double check
				for i, (a, b) in enumerate(x.wu_ranges):
					print (symbol.value)
					mpdb.append(mpgen_mpdb.MP_WR_ONCE_UNKNOWN(
													"%s.%i" % (symbol.name, i),
													"%x" % (int(symbol.value, base = 16) + a),
													"%x" % (b - a),
													writers,
													x.attrs,
													"0x0"))

			if x.mp_type is mpgen_mpdb.WR_ONCE_UNKNOWN_SPLIT :
				symbol = kernel.get_symbol(x.name)
				if (symbol is None) : continue

				end_addr = int(symbol.value, 16) + int(symbol.size, 16)
				addrs = range(int(symbol.value, 16), end_addr, x.split_step)
				for i, addr in enumerate(addrs):
					mpdb.append(mpgen_mpdb.MP_WR_ONCE_UNKNOWN(
													"%s.0x%x" % (symbol.name, i * x.split_step),
													"%x" % (addr),
													"%x" % (min(x.split_step, end_addr - addr)),
													writers,
													x.attrs,
													"0x0"))
				# fill if reserve size is given
				if x.split_reserve_size:
					addrs = range(addrs[-1], int(symbol.value, 16) + x.split_reserve_size, x.split_step)
					for i, addr in enumerate(addrs):
						print ("filler addr %x" % (addr))
						mpdb.append(mpgen_mpdb.MP_WR_ONCE_UNKNOWN(
														"%s.filler.%i" % (symbol.name, i),
														"%x" % (addr),
														"0",
														writers,
														"MP_SECTION_ATTRIBUTE_NRTIC | MP_SECTION_ATTRIBUTE_NQHEE",
														"0"))

			if x.mp_type is mpgen_mpdb.WR_AUTH_WRITER :
				symbol = kernel.get_symbol(x.name)
				if (symbol is None) : continue
				if (not writers):
					print("WARNING: AW: mpgen authorized writer not found: %s (skiping this asset)" % (x.writer), file = sys.stderr)
					continue
				mpdb.append(mpgen_mpdb.MP_WR_AUTH_WRITERS(
												symbol.name,
												symbol.value,
												symbol.size,
												x.attrs,
												writers))

			if x.mp_type is mpgen_mpdb.KCONFIG :
				parameter = config.get_parameter(x.name)
				if (parameter is None) : continue
				mpdb.append(mpgen_mpdb.MP_KCONFIG(x.name, parameter.value))

		except mpgen_kernel.ExceptionSymbolNotFound as e:  # symbol not found is just a warning, report and continue
			catalog.mp_warning("No sym. `%s`" % (e))
			# mp_warning prints the above print ("WARNING: could not find symbol: `%s`" % (e), file = sys.stderr)
		except mpgen_kernel.ExceptionDwarfParsingFailed as e:  # symbol not found is just a warning, report and continue
			catalog.mp_warning("No dwarf `%s`" % (e))
			# mp_warning prints the above print ("WARNING: could not find symbol: `%s`" % (e), file = sys.stderr)

	return mpdb

