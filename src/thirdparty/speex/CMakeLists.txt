cmake_minimum_required( VERSION 3.16 )

add_module( "lib" "libspeexdsp" "" ${FOLDER_CONTEXT} TRUE FALSE )

start_sources()

add_sources( SOURCE_GROUP "Resampler"
    "resample.cpp"
    "include/arch.h"
    "include/resample_sse.h"
    "include/speex_resampler.h"
    "include/config.h"
)

end_sources()

target_compile_definitions( ${PROJECT_NAME} PRIVATE
    "OUTSIDE_SPEEX"
    "RANDOM_PREFIX=r5sdk"
)

target_include_directories( ${PROJECT_NAME} PRIVATE
    "${ENGINE_SOURCE_DIR}/thirdparty/speex/include" 
)

thirdparty_suppress_warnings()