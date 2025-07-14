# Add your desired shader directories in the directory of this CMakeLists.txt file
# Everything in this directory will be copied to Falcor's shader folder
# The copying process will retain the structure of this directory in Falcor's shader folder
# You may add some defines here for your program for identifying your shaders in Falcor's shader folder
# You can do this with the target_compile_definitions(<your-executable-name>) function
# You should also add your shader sources to your target by using
# target_sources(<your-executable-name> ...)
# Use get_shader_files_from_directory to get all shader files in a directory

# Framework shaders sources
get_shader_files_from_directories(GRAPHEX_DIR_SHADER_FILES GraphEx)

target_sources(${GRAPHEX_TARGET_NAME} PRIVATE
    ${GRAPHEX_DIR_SHADER_FILES}
    GraphEx.slang
)

target_compile_definitions(${GRAPHEX_TARGET_NAME} PUBLIC
    GRAPHEX_SHADER_DIR="GraphEx"
)

# Add your program shader directory defines & sources here
