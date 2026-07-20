execute_process(
  COMMAND "${GIT_EXECUTABLE}" rev-parse HEAD
  WORKING_DIRECTORY "${SRC_DIR}"
  OUTPUT_VARIABLE GIT_HASH
  OUTPUT_STRIP_TRAILING_WHITESPACE)

file(WRITE "${OUT_FILE}" "namespace cvc { const char *g_gitversion = \"${GIT_HASH}\"; }\n")
