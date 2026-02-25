# Create the command to run the code generator
set (CODEGEN_EXE $<TARGET_FILE_DIR:MikanClientCodeGen>/$<TARGET_NAME:MikanClientCodeGen>)
add_custom_target(MikanTypeScriptCodeGen
					WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}"
					COMMAND "${CODEGEN_EXE}" "${CMAKE_CURRENT_LIST_DIR}/TypeScriptCodeGenConfig.json")
set_target_properties(MikanTypeScriptCodeGen PROPERTIES FOLDER Programs)

# Note: TypeScript doesn't need to be built like C#, so no dependencies are added
# The generated .ts files are compiled separately via npm/tsc