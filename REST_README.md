REST support
============

xDPd's can export its management APIs via REST using the `rest` plugin.

Current features:

* List xDPd general information, like the ID of the system, version or the driver used
* List the ports in the system
* List the logical switch instances(LSIs) in the system
* Show the details of an LSI
* List the flows in an LSI table
* Other

Future plans:

* Group tables
* Improve formatting of flows
* More management routines

Supported platforms (drivers)
=============================

The REST support is the driver agnostic part of xDPd's, the Control and Management Module (CMM), so works across all drivers.

Requirements
============

- Boost 2.54 or higher, boost-threads, boost-regex and boost-system. It is recommended to install all the boost libraries.

- Common python packages: cmd, getopt and urllib/urllib2

How to build
============

To build support for rest:

	sh# ../configure --with-plugins="rest, other" 
	sh# make
	sh# make install

This will also install `xcli` tool, typically in `/usr/local/bin`.

REST API
========

The REST API listens typically in port `5757`. The API returns JSON replies. There is a special page for the user convenience that lists the APIs in HTML. Pointing a browser to the root:

	http://127.0.0.1:5757/
	http://127.0.0.1:5757/index.html

xcli command line tool
======================

`xcli` is a python script that consumes xDPd's REST API. `xcli` has two modes of operation:

* Command or script mode, specifying `-c "command"`
* Interactive mode: a CLI, supporting multiple targets, so multiple local and remote xDPd instances, at the same time. The default if `-c` is not defined.

For usage information:

	xcli --help


Examples (using xcli)
=====================

Non-interactive
---------------

List ports:

	xcli -c "show ports"

Output:
	{
	    "ports" : [
		"eth0",
		"eth1",
		"eth2",
		"eth3",
		"eth4",
		"veth0",
		"veth1",
		"veth2",
		"veth3",
		"veth4",
		"veth5",
		"veth6",
		"veth7"
	    ]
	}


Show detailed information for `eth1` port:

	xcli -c "show port eth1"

Output:

	{
	    "name" : "eth1",
	    "is-vlink" : "no",
	    "mac-address" : "52:54:00:2a:1b:fc",
	    "up" : "yes",
	    "forward-packets" : "yes",
	    "drop-received" : "no",
	    "no-flood" : "no",
	    "is-blacklisted" : "no",
	    "type" : "physical",
	    "link" : "up",
	    "statistics" : {
		"rx_packets" : 6672,
		"tx_packets" : 6669,
		"rx_bytes" : 646228,
		"tx_bytes" : 640906,
		"rx_dropped" : 0,
		"tx_dropped" : 0,
		"rx_errors" : 0,
		"tx_errors" : 0,
		"rx_frame_err" : 0,
		"rx_over_err" : 0,
		"rx_crc_err" : 0,
		"collisions" : 0
	    },
	    "openflow" : {
		"attached-dpid" : 256,
		"generate-pkt-in" : "yes",
		"port-num" : 1
	    },
	    "queues" : {
		"0" : {
		    "id" : 0,
		    "length" : 2048,
		    "statistics" : {
			"tx-pkts" : 6669,
			"tx-bytes" : 640906
		    }
		},
		"1" : {
		    "id" : 1,
		    "length" : 2048,
		    "statistics" : {
			"tx-pkts" : 0,
			"tx-bytes" : 0
		    }
		},
		"2" : {
		    "id" : 2,
		    "length" : 2048,
		    "statistics" : {
			"tx-pkts" : 0,
			"tx-bytes" : 0
		    }
		},
		"3" : {
		    "id" : 3,
		    "length" : 2048,
		    "statistics" : {
			"tx-pkts" : 0,
			"tx-bytes" : 0
		    }
		},
		"4" : {
		    "id" : 4,
		    "length" : 2048,
		    "statistics" : {
			"tx-pkts" : 0,
			"tx-bytes" : 0
		    }
		},
		"5" : {
		    "id" : 5,
		    "length" : 2048,
		    "statistics" : {
			"tx-pkts" : 0,
			"tx-bytes" : 0
		    }
		},
		"6" : {
		    "id" : 6,
		    "length" : 2048,
		    "statistics" : {
			"tx-pkts" : 0,
			"tx-bytes" : 0
		    }
		},
		"7" : {
		    "id" : 7,
		    "length" : 2048,
		    "statistics" : {
			"tx-pkts" : 0,
			"tx-bytes" : 0
		    }
		}
	    }
	}

Show detailed information for `eth1` port:

	xcli -c "show list dp0 table 0 flows"

Output (Warning: this has to be fixed, as of ROFL issue bisdn/rofl-core#13):

	{
	    "table" : {
		"id" : 0,
		"flows" : [
		    " p:0 cookie:0 pkt_count:7737 {matches<cofmatch ofp-version:0 >\n  <coxmatches #matches:2 >\n    <coxmatch oxm-id: 0x80000004 >\n      <cmemory: data:0x7fffcc001260 datalen:8 >\n        0000: 80 00 00 04 00 00 00 02   \n      <in-port: 2 >\n    <coxmatch oxm-id: 0x8000070c >\n      <cmemory: data:0x7fffcc001260 datalen:16 >\n        0000: 80 00 07 0c fe 54 00 2a   1b fc ff ff ff ff ff ff   \n      <eth-dst: fe:54:00:2a:1b:fc/ff:ff:ff:ff:ff:ff >\n }\n {, instructions<cofintructions ofp-version:0 >\n  <cofinstruction_apply_actions >\n    <cofinstruction_actions >\n      <cofinstruction type: 0x4 length: 24 >\n        <cofactions ofp-version:3 #actions:1 >\n          <cofaction_output port-no: 0x1 max-len: 0xffe5 >\n            <cofaction ofp-version:3 type:0x0 length:4  >\n }\n",
		    " p:0 cookie:0 pkt_count:7737 {matches<cofmatch ofp-version:0 >\n  <coxmatches #matches:2 >\n    <coxmatch oxm-id: 0x80000004 >\n      <cmemory: data:0x7fffcc001230 datalen:8 >\n        0000: 80 00 00 04 00 00 00 01   \n      <in-port: 1 >\n    <coxmatch oxm-id: 0x8000070c >\n      <cmemory: data:0x7fffcc001250 datalen:16 >\n        0000: 80 00 07 0c ee 6e cd a7   32 f5 ff ff ff ff ff ff   \n      <eth-dst: ee:6e:cd:a7:32:f5/ff:ff:ff:ff:ff:ff >\n }\n {, instructions<cofintructions ofp-version:0 >\n  <cofinstruction_apply_actions >\n    <cofinstruction_actions >\n      <cofinstruction type: 0x4 length: 24 >\n        <cofactions ofp-version:3 #actions:1 >\n          <cofaction_output port-no: 0x2 max-len: 0xffe5 >\n            <cofaction ofp-version:3 type:0x0 length:4  >\n }\n"
		]
	    }
	}




Interactive
-----------

Show the LSI information for a local xDPd instance (lsi `dp0`) and a remote xDPd in another host (lsi `remote_dp0`):

Interactive:

	marc@dev:~/$ xcli
	xDPd v0.7 command line tool

	Using target '[local] 127.0.0.1:5757'

	xcli(local)>add_target remote 172.16.250.100:5757
	xcli(local)>targets
	[remote] 172.16.250.100:5757
	[local] 127.0.0.1:5757
	xcli(local)>show lsi dp0
	{
	    "name" : "dp0",
	    "dpid" : 256,
	    "of_version" : "1.2",
	    "num-of-tables" : 8,
	    "miss-send-len" : 128,
	    "num-of-buffers" : 512,
	    "attached-ports" : {
		"1" : "eth1",
		"2" : "veth0",
		"3" : "veth2",
		"4" : "veth4",
		"5" : "veth6"
	    },
	    "tables" : {
		"0" : {
		    "number" : 0,
		    "matching-algorithm" : 0,
		    "num-of-entries" : 2,
		    "max-entries" : 4294967295,
		    "statistics" : {
			"pkts-looked-up" : 13914,
			"pkts-matched" : 13868
		    }
		},
		"1" : {
		    "number" : 1,
		    "matching-algorithm" : 0,
		    "num-of-entries" : 0,
		    "max-entries" : 4294967295,
		    "statistics" : {
			"pkts-looked-up" : 0,
			"pkts-matched" : 0
		    }
		},
		"2" : {
		    "number" : 2,
		    "matching-algorithm" : 0,
		    "num-of-entries" : 0,
		    "max-entries" : 4294967295,
		    "statistics" : {
			"pkts-looked-up" : 0,
			"pkts-matched" : 0
		    }
		},
		"3" : {
		    "number" : 3,
		    "matching-algorithm" : 0,
		    "num-of-entries" : 0,
		    "max-entries" : 4294967295,
		    "statistics" : {
			"pkts-looked-up" : 0,
			"pkts-matched" : 0
		    }
		},
		"4" : {
		    "number" : 4,
		    "matching-algorithm" : 0,
		    "num-of-entries" : 0,
		    "max-entries" : 4294967295,
		    "statistics" : {
			"pkts-looked-up" : 0,
			"pkts-matched" : 0
		    }
		},
		"5" : {
		    "number" : 5,
		    "matching-algorithm" : 0,
		    "num-of-entries" : 0,
		    "max-entries" : 4294967295,
		    "statistics" : {
			"pkts-looked-up" : 0,
			"pkts-matched" : 0
		    }
		},
		"6" : {
		    "number" : 6,
		    "matching-algorithm" : 0,
		    "num-of-entries" : 0,
		    "max-entries" : 4294967295,
		    "statistics" : {
			"pkts-looked-up" : 0,
			"pkts-matched" : 0
		    }
		},
		"7" : {
		    "number" : 7,
		    "matching-algorithm" : 0,
		    "num-of-entries" : 0,
		    "max-entries" : 4294967295,
		    "statistics" : {
			"pkts-looked-up" : 0,
			"pkts-matched" : 0
		    }
		}
	    }
	}
	xcli(local)>switch_target remote
	xcli(remote)>show lsi remote_dp0 
	{
	    "name" : "remote_dp0",
	    "dpid" : 512,
	    "of_version" : "1.3",
	    "num-of-tables" : 8,
	    "miss-send-len" : 128,
	    "num-of-buffers" : 512,
	    "attached-ports" : {
		"1" : "eth1",
		"2" : "eth2"
	    },
	    "tables" : {
		"0" : {
		    "number" : 0,
		    "matching-algorithm" : 0,
		    "num-of-entries" : 25,
		    "max-entries" : 4294967295,
		    "statistics" : {
			"pkts-looked-up" : 13,
			"pkts-matched" : 11
		    }
		},
		"1" : {
		    "number" : 1,
		    "matching-algorithm" : 0,
		    "num-of-entries" : 0,
		    "max-entries" : 4294967295,
		    "statistics" : {
			"pkts-looked-up" : 0,
			"pkts-matched" : 0
		    }
		},
		"2" : {
		    "number" : 2,
		    "matching-algorithm" : 0,
		    "num-of-entries" : 0,
		    "max-entries" : 4294967295,
		    "statistics" : {
			"pkts-looked-up" : 0,
			"pkts-matched" : 0
		    }
		},
		"3" : {
		    "number" : 3,
		    "matching-algorithm" : 0,
		    "num-of-entries" : 0,
		    "max-entries" : 4294967295,
		    "statistics" : {
			"pkts-looked-up" : 0,
			"pkts-matched" : 0
		    }
		},
		"4" : {
		    "number" : 4,
		    "matching-algorithm" : 0,
		    "num-of-entries" : 0,
		    "max-entries" : 4294967295,
		    "statistics" : {
			"pkts-looked-up" : 0,
			"pkts-matched" : 0
		    }
		},
		"5" : {
		    "number" : 5,
		    "matching-algorithm" : 0,
		    "num-of-entries" : 0,
		    "max-entries" : 4294967295,
		    "statistics" : {
			"pkts-looked-up" : 0,
			"pkts-matched" : 0
		    }
		},
		"6" : {
		    "number" : 6,
		    "matching-algorithm" : 0,
		    "num-of-entries" : 0,
		    "max-entries" : 4294967295,
		    "statistics" : {
			"pkts-looked-up" : 0,
			"pkts-matched" : 0
		    }
		},
		"7" : {
		    "number" : 7,
		    "matching-algorithm" : 0,
		    "num-of-entries" : 0,
		    "max-entries" : 4294967295,
		    "statistics" : {
			"pkts-looked-up" : 0,
			"pkts-matched" : 0
		    }
		}
	    }
	}
	xcli(remote)>

