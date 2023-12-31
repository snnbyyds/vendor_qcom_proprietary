{# Copyright (c) 2017-2018 Qualcomm Technologies, Inc.
	All Rights Reserved.
	Confidential and Proprietary - Qualcomm Technologies, Inc. #}
{% include  'rtic_mp_header.template.h' %}

/**
 *  Dynamic RTIC MP container/ the top level RTIC MP structure
 *  (Reserve space for standard and dynamic features)
 */
typedef struct {
	mp_rtic_header_t header;
	mp_feature_discovery_t featture_discovery[{{len(dynamic_features)}}]; // feature discovery

	// legacy features (KV is empty)
	mp_attestation_ro_section_t read_only[/*!!!*/{{len(ro)}}/*!!!*/]; // actual number of RO section attestations
	mp_attestation_write_once_t wo_known[/*!!!*/{{len(wk)}}/*!!!*/];
	mp_attestation_write_once_t wo_unknown[/*!!!*/{{len(wu)}}/*!!!*/];
	mp_attestation_black_listed_t black_listed[/*!!!*/{{len(aw)}}/*!!!*/];

	// dynamic features - start
{%- for ftype, value in dynamic_features %}
	{{ftype}} feature_item_{{loop.index}};
{%- endfor %}
	// dynamic features - end

}__attribute__((__packed__)) mp_var_rtic_mp_t;

/**
 * Declare RTIC MP STRUCTURE
 */
const mp_var_rtic_mp_t rtic_mp __attribute__ ((aligned (/* 1MB aligned*/0x100000))) = { /**/

	{ /* mp_rtic_header_t header; */

		/* unsigned char const marker_str[50]; // Magic RTIC header string */
		/*!!!*/RTIC_MP_MARKER_STRING/*!!!*/,

		/* unsigned int const mpgen_interface_ver; // MPGen interface version. !!! Can't be 0 as can be used as flag that MP is not initialized on TA/HP side */
		/*!!!*/RTIC_MPGEN_INTERFACE_VERSION_GEN(RTIC_MPGEN_INTERFACE_VERSION, RTIC_MPGEN_INTERFACE_VERSION_MINOR)/*!!!*/,

		/* unsigned char const mpgen_ver[8]; // MPGen tool version. */
		/*!!!*/RTIC_MPGEN_VERSION/*!!!*/,

		/* whole_rtic_size */
		/*!!!*/sizeof(mp_var_rtic_mp_t) /*!!!*/,

		/* long long unsigned int const created_tmstmp; // 64bit time stamp from the beginning of epoch */
		/*!!!*/{{created_tmstmp}}/*!!!*/,

		{ /* mp_hlos_kernel_specs_t */
			/*char linux_banner_content[512]; // content of the kernel version string */
			/*!!!*/"{{banner}}", /*!!!*/
			/* long long unsigned int linux_banner_el1_va; // el1 va address of the kernel banner */
			/*!!!*/0x{{kernel.get_symbol("linux_banner").value}},/*!!!*/
			/* unsigned int linux_banner_size; */
			/*!!!*/0x{{kernel.get_symbol("linux_banner").size}},/*!!!*/
			/* long long unsigned int const linux_kernel_module_base_va; // base virtual address of the kernel's modules section */
			/*!!!*/{# TODO: populate this field! #} 0x0,/*!!!*/
			/* unsigned int const linux_kernel_module_size; // size of the kernel's module section */
			/*!!!*/{# TODO: populate this field! #} 0x0,/*!!!*/
			/* long long unsigned int vectors_el1_va; // el1 va address of hlos exception table */
			/*!!!*/{{vectors}},/*!!!*/
			/* long long unsigned int const kernel_offset_le; // kernel offset */
			/*!!!*/0x{{kernel_offset_le}},/*!!!*/
			/* long long unsigned int const kernel_size_le; // kernel size */
			/*!!!*/0x{{kernel_size_le}},/*!!!*/
			/* unsigned char  const is_checkable_kernel; // // false if mpgen detects error/ unsupported kernel features */
			/*!!!*/{{mpdb.catalog.is_checkable_kernel}}, /*!!!*/
			/* unsigned char const mpgen_message[239],    // readable mpgen error/warning */
			/*!!!*/"{{mpdb.catalog.mp_warning_msg}}",/*!!!*/
			/* unsigned long long int const linux_entry_task_va; // __entry_task */
			0x{{linux_entry_task_va}},
			/* unsigned long long int const linux_per_cpu_offset_va; // __per_cpu_offset */
			0x{{linux_per_cpu_offset_va}},
		}, /* mp_hlos_kernel_specs_t */

		{ /* Offset table  - feature locator */

			{ /* MP_ATTESTATION_OU - uses feature discovery */
				/* unsigned int offset; // offset from MP base */
				0x0,
				/* unsigned int size; // whole size of given structure block */
				0x0,},

			{ /* MP_ATTESTATION_QR - uses feature discovery */
				/* unsigned int offset; // offset from MP base */
				0x0,
				/* unsigned int size; // whole size of given structure block */
				0x0,},

			{ /* MP_ATTESTATION_KV - placeholder */
				/* unsigned int offset; // offset from MP base */
				0x0,
				/* unsigned int size; // whole size of given structure block */
				0x0,},

			{ /* MP_ATTESTATION_RO */
				/* unsigned int offset; // offset from MP base */
				(unsigned int)(long unsigned int)&((mp_var_rtic_mp_t *) 0x0)->read_only /**/,
				/* unsigned int size; // whole size of given structure block */
				sizeof(((mp_var_rtic_mp_t *) 0x0)->read_only) /**/,},

			{ /* MP_ATTESTATION_WK */
				/* unsigned int offset; // offset from MP base */
				(unsigned int)(long unsigned int)&((mp_var_rtic_mp_t *) 0x0)->wo_known /**/,
				/* unsigned int size; // whole size of given structure block */
				sizeof(((mp_var_rtic_mp_t *) 0x0)->wo_known) /**/,},

			{ /* MP_ATTESTATION_WU */
				/* unsigned int offset; // offset from MP base */
				(unsigned int)(long unsigned int)&((mp_var_rtic_mp_t *) 0x0)->wo_unknown /**/,
				/* unsigned int size; // whole size of given structure block */
				sizeof(((mp_var_rtic_mp_t *) 0x0)->wo_unknown) /**/,},

			{ /* MP_ATTESTATION_AW */
				/* unsigned int offset; // offset from MP base */
				(unsigned int)(long unsigned int)&((mp_var_rtic_mp_t *) 0x0)->black_listed /**/,
				/* unsigned int size; // whole size of given structure block */
				sizeof(((mp_var_rtic_mp_t *) 0x0)->black_listed) /**/,},
		}, /* Offset table  - feature locator */

		/* unsigned int const num_discovery_items; // number of discovery items */
		/*!!!*/{{len(dynamic_features)}},/*!!!*/
		/* unsigned int const featture_discovery_size; // feature discovery size (redundant) */
		/*!!!*/{{len(dynamic_features)}} * sizeof(mp_feature_discovery_t),/*!!!*/
		/* mp_feature_discovery_t featture_discovery[0]; // feature discovery marker */

	}, /* mp_rtic_header_t header; */

	/* ========= FEATURE DISCOVERY ========= */
	{ /* mp_feature_discovery_t featture_discovery[0]; // feature discovery marker */
	{% for ftype, value in dynamic_features %}
		{ /* {{ftype}} - {{loop.index}} */
		/* char const name[32]; // feature name - string representation for a given type - this also is feature id */
		"{{ftype}}" /**/,
		/* unsigned int offset; // offset from MP base */
		(unsigned int)(long unsigned int) &((mp_var_rtic_mp_t *) 0x0)->feature_item_{{loop.index}} /**/,
		/* unsigned int size; // whole size of given structure block */
		sizeof(((mp_var_rtic_mp_t *) 0x0)->feature_item_{{loop.index}}) /**/,},
	{% endfor %}
	},

	/* ========= LEGACY FEATURE DATA ========= */
	/* OU - Empty */
	/* QR - Empty */
	/* KV - Empty */

	{ /* MP_ATTESTATION_RO */
		{% for mp in ro %}
		{ /* item */
			{ /* section */
				/* unsigned char const name[20]; */
				"{{mp.name}}",
				/* long long unsigned int start_addr; */
				0x{{mp.vaddr}},
				/* unsigned int length; */
				0x{{mp.size}},
				/* unsigned int const attributes; // memory types, etc, used by QHEE */
				{{mp.attrs}},
			}, /* section */
			/* unsigned char sha256[32]; */
			{0x{{mp.sha256.replace(':', ',0x')}}},  /**/
		}, /* item */
		{% endfor %}
	}, /* MP_ATTESTATION_RO */

	{ /* MP_ATTESTATION_WK */
	  	  {% for mp in wk %}
	  	  { /* item */
			{ /* section */
				/* unsigned char const name[20]; */
				"{{mp.name}}",
				/* long long unsigned int start_addr; */
				0x{{mp.vaddr}},
				/* unsigned int length; */
				0x{{mp.size}},
				/* unsigned int const attributes; // memory types, etc, used by QHEE */
				{{mp.attrs}},
			}, /* section */
			/* unsigned char sha256[32]; */
			{0x{{mp.sha256.replace(':', ',0x')}}},  /**/
			/* mp_authorized_writers_t const writers; */
			// generate auth list
			{ {%- for el in mp.writers %}
				 {0x{{el.value}}, 0x{{el.size}}}, /* {{el.name}} */
			  {%- endfor %}
			}
		}, /* item */
	  	{% endfor %}
	}, /* MP_ATTESTATION_WK */

	{ /* MP_ATTESTATION_WU */
		{% for mp in wu %}
		{ /* item */
			{ /* section */
				/* unsigned char const name[20]; */
				"{{mp.name}}",
				/* long long unsigned int start_addr; */
				0x{{mp.vaddr}},
				/* unsigned int length; */
				0x{{mp.size}},
				/* unsigned int const attributes; // memory types, etc, used by QHEE */
				{{mp.attrs}},
			}, /* section */
			/* unsigned char sha256[32]; */
			{0x0},  /**/
			/* mp_authorized_writers_t const writers; */
			// generate auth list
			{ {%- for el in mp.writers %}
				 {0x{{el.value}}, 0x{{el.size}}}, /* {{el.name}} */
			  {%- endfor %}
			}
		}, /* item */
		{% endfor %}
	}, /* MP_ATTESTATION_WU */

	{ /* MP_ATTESTATION_AW */
		{% for mp in aw %}
		{ /* item */
			{ /* section */
				/* unsigned char const name[20]; */
				"{{mp.name}}",
				/* long long unsigned int start_addr; */
				0x{{mp.vaddr}},
				/* unsigned int length; */
				0x{{mp.size}},
				/* unsigned int const attributes; // memory types, etc, used by QHEE */
				{{mp.attrs}},
			}, /* section */
			/* mp_authorized_writers_t const writers; */
			// generate auth list
			{ {%- for el in mp.writers %}
				 {0x{{el.value}}, 0x{{el.size}}}, /* {{el.name}} */
			  {%- endfor %}
			}
		}, /* item */
		{% endfor %}
	}, /* MP_ATTESTATION_AW */

	/* ========= DYNAMIC ITEMS ========= */
	{%- for ftype, value in dynamic_features %}
	/* {{ftype}} feature_item_{{loop.index}} */
	{{value}},
	{%- endfor %}
	/* ========= DYNAMIC ITEMS ========= */

};

