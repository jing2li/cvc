# Replicates AFF's lib/Makefile interface-generation rule:
#   cat interface.prologue > lhpc-aff.h
#   for each part: print a comment header, then append the part with lines
#     containing MARK_ (the header-guard tokens) stripped
#   cat interface.epilogue >> lhpc-aff.h
#
# The part list is hardcoded here (matching lib/Makefile's interface.parts)
# rather than passed in via -D, since semicolon-separated list values get
# reinterpreted as separate COMMAND invocations by add_custom_command.
set(INTERFACE_PARTS aff.h node.h tree.h stable.h treap.h md5.h coding.h
                    alloc.h)

file(READ "${AFF_LIB_DIR}/interface.prologue" _prologue)
file(WRITE "${OUTPUT_HEADER}" "${_prologue}")

foreach(_part IN LISTS INTERFACE_PARTS)
  file(APPEND "${OUTPUT_HEADER}" "/* ${_part} */\n")
  file(STRINGS "${AFF_LIB_DIR}/${_part}" _lines)
  foreach(_line IN LISTS _lines)
    if(NOT _line MATCHES "MARK_")
      file(APPEND "${OUTPUT_HEADER}" "${_line}\n")
    endif()
  endforeach()
endforeach()

file(READ "${AFF_LIB_DIR}/interface.epilogue" _epilogue)
file(APPEND "${OUTPUT_HEADER}" "${_epilogue}")
