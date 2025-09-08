
executable(
    'std_auto_tune',
    'auto_tune_runtime.cpp',
    dependencies:hwy_dep,
    override_options: ['cpp_std=c++20']
)

if contrib_enabled
    executable(
        'vqsort_auto_tune',
        'auto_tune_runtime.cpp',
        dependencies: hwy_contrib_dep,
        cpp_args: '-DHWY_AUTOTUNE_USE_VQSORT',
        override_options: ['cpp_std=c++20']
    )
endif