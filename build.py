#!/usr/bin/env python3
import os
import sys
import yaml
import subprocess
import shutil
from pathlib import Path
import argparse
from typing import List, Dict, Any

class BuildSystem:
    def __init__(self, config_path: str = "amslah.yaml", variables: Dict[str, str] = None):
        self.config_path = config_path
        self.config = self.load_config()
        self.amslah_path = Path(__file__).parent.absolute()
        self.project_path = Path.cwd()
        self.build_path = self.project_path / "build"
        self.src_path = self.build_path / "src"
        self.obj_path = self.build_path / "obj"
        self.makefile_path = self.build_path / "generated.mk"
        self.variables = variables or {}
        
    def load_config(self) -> Dict[str, Any]:
        """Load YAML configuration file"""
        try:
            with open(self.config_path, 'r') as f:
                return yaml.safe_load(f)
        except FileNotFoundError:
            print(f"Error: Configuration file {self.config_path} not found")
            sys.exit(1)
        except yaml.YAMLError as e:
            print(f"Error parsing YAML configuration: {e}")
            sys.exit(1)
    
    def get_mcu_flags(self) -> Dict[str, List[str]]:
        """Get MCU-specific compiler and linker flags"""
        mcu = self.config.get('mcu', 'SAMD21J18A')
        
        common_flags = [
            '-mthumb', '-DDEBUG', '-Os', '-ffunction-sections', 
            '-mlong-calls', '-Wall', '-g3', '-fstack-usage',
            '-c', f'-D__{mcu}__', '-mfp16-format=ieee',
            '-specs=nano.specs', '-specs=nosys.specs'
        ]
        
        if 'SAMD21' in mcu:
            cpu_flags = [
                '-mcpu=cortex-m0plus',
                '-mfloat-abi=soft', '-msoft-float', '-fsingle-precision-constant'
            ]
            linker_flags = [
                '-mfloat-abi=soft', '-mthumb', '-msoft-float',
                f'-T{self.amslah_path}/core/samd21j18a_flash.ld',
                '-Wl,--gc-sections', '-mcpu=cortex-m0plus'
            ]
        else:  # SAMD51
            cpu_flags = [
                '-mcpu=cortex-m4',
                '-mfloat-abi=hard', '-mfpu=fpv4-sp-d16', '-fsingle-precision-constant'
            ]
            linker_flags = [
                '-mfloat-abi=hard', '-mthumb', '-mfpu=fpv4-sp-d16',
                f'-T{self.amslah_path}/core/samd51n20a_flash.ld',
                '-Wl,--gc-sections', '-mcpu=cortex-m4', '-mtune=cortex-m4'
            ]
            
        return {
            'cflags': common_flags + cpu_flags,
            'lflags': linker_flags + ['-lm', '-specs=nano.specs', '-specs=nosys.specs']
        }
    
    def get_include_paths(self) -> List[str]:
        """Generate include paths from configuration"""
        # Since everything is flattened, we only need core paths
        includes = [
            f'{self.amslah_path}/core',
            f'{self.amslah_path}/config', 
            f'{self.amslah_path}/freertos/include',
            f'{self.amslah_path}/freertos/portable_samd51',
            f'{self.amslah_path}/extra'
        ]
        
        # All headers will be in build/src, so that's added in generate_makefile
        
        return [f'-I{path}' for path in includes]
    
    def run_hooks(self) -> bool:
        """Run preprocessing hooks (like zemi)"""
        hooks = self.config.get('hooks', '')
        if not hooks:
            return True
        
        # Substitute variables like __CF__ -> value of CF
        hooks_expanded = self.substitute_variables(hooks)
        print(f"Running hooks: {hooks_expanded}")
        
        try:
            result = subprocess.run(hooks_expanded, shell=True, check=True, 
                                  capture_output=True, text=True)
            if result.stdout:
                print(result.stdout)
            return True
        except subprocess.CalledProcessError as e:
            print(f"Hook failed: {e}")
            if e.stderr:
                print(e.stderr)
            return False
    
    def substitute_variables(self, text: str) -> str:
        """Replace __VARNAME__ with variable values"""
        import re
        
        # Find all __VARNAME__ patterns
        pattern = r'__([A-Za-z_][A-Za-z0-9_]*)__'
        
        def replace_var(match):
            var_name = match.group(1)
            # Look for variable in our variables dict, environment, or default to empty
            if var_name in self.variables:
                value = self.variables[var_name]
            elif var_name in os.environ:
                value = os.environ[var_name]
            else:
                value = ""  # Default to empty like original Makefile
            return value
        
        return re.sub(pattern, replace_var, text)
    
    def find_source_files(self) -> Dict[str, List[str]]:
        """Find all source files to compile"""
        # First, check if zemi_output exists (Zemi processed files take priority)
        zemi_output_path = self.project_path / "zemi_output"
        
        # Collect all source files from various locations
        all_cpp_files = {}  # filename -> full path
        all_c_files = {}    # filename -> full path
        all_h_files = {}    # filename -> full path
        
        # Priority 1: Check zemi_output directory first
        if zemi_output_path.exists():
            for f in zemi_output_path.glob('*.cpp'):
                basename = f.name
                if basename in all_cpp_files:
                    raise AssertionError(f"Filename conflict in zemi_output: {basename}")
                all_cpp_files[basename] = f
            for f in zemi_output_path.glob('*.c'):
                basename = f.name
                if basename in all_c_files:
                    raise AssertionError(f"Filename conflict in zemi_output: {basename}")
                all_c_files[basename] = f
            for f in zemi_output_path.glob('*.h'):
                basename = f.name
                if basename in all_h_files:
                    raise AssertionError(f"Filename conflict in zemi_output: {basename}")
                all_h_files[basename] = f
        
        # Priority 2: Search other directories for non-Zemi files
        dirs = ['.']
        
        # Add library directories
        for lib in self.config.get('libs', []):
            lib_path = Path(lib)
            if not lib_path.is_absolute():
                lib_path = self.project_path / lib_path
            if lib_path.exists():
                dirs.append(str(lib_path))
        
        # Add subdirectories but exclude build, zemi_output, and excluded dirs
        exclude = set(self.config.get('exclude', []))
        exclude.add('build')
        exclude.add('zemi_output')
        exclude.add('output')  # Also exclude old output directory
        
        for item in self.project_path.iterdir():
            if item.is_dir() and item.name not in exclude:
                dirs.append(str(item))
        
        # Search directories for source files
        for dir_path in dirs:
            dir_p = Path(dir_path)
            if dir_p.exists():
                # Skip Zemi source files (.cppz, .hz, .cz)
                for f in dir_p.glob('*.cpp'):
                    basename = f.name
                    if basename not in all_cpp_files:
                        if f.with_suffix('.cppz').exists():
                            continue  # Skip, will be handled by Zemi
                        all_cpp_files[basename] = f
                    else:
                        # Check for conflict
                        if all_cpp_files[basename] != f:
                            raise AssertionError(f"Filename conflict: '{basename}' found in both '{all_cpp_files[basename].parent}' and '{f.parent}'")
                
                for f in dir_p.glob('*.c'):
                    basename = f.name
                    if basename not in all_c_files:
                        if f.with_suffix('.cz').exists():
                            continue  # Skip, will be handled by Zemi
                        all_c_files[basename] = f
                    else:
                        # Check for conflict
                        if all_c_files[basename] != f:
                            raise AssertionError(f"Filename conflict: '{basename}' found in both '{all_c_files[basename].parent}' and '{f.parent}'")
                
                for f in dir_p.glob('*.h'):
                    basename = f.name
                    if basename not in all_h_files:
                        if f.with_suffix('.hz').exists():
                            continue  # Skip, will be handled by Zemi
                        all_h_files[basename] = f
                    else:
                        # Check for conflict
                        if all_h_files[basename] != f:
                            raise AssertionError(f"Filename conflict: '{basename}' found in both '{all_h_files[basename].parent}' and '{f.parent}'")
        
        # Add core AMSLAH files
        mcu = self.config.get('mcu', 'SAMD21J18A')
        core_files = [
            f'{self.amslah_path}/core/gpio.c',
            f'{self.amslah_path}/core/util.c',
            f'{self.amslah_path}/core/uart.c',
            f'{self.amslah_path}/core/eeprom.c',
            f'{self.amslah_path}/core/spi.c',
            f'{self.amslah_path}/core/i2c.c',
            f'{self.amslah_path}/core/pwm.c',
            f'{self.amslah_path}/core/adc.c',
            f'{self.amslah_path}/core/dac.c',
            f'{self.amslah_path}/core/eic.c',
            f'{self.amslah_path}/core/watchdog.c',
            f'{self.amslah_path}/core/sercoms.c',
            f'{self.amslah_path}/core/serial.c',
            f'{self.amslah_path}/core/printf.c',
        ]
        
        if 'SAMD21' in mcu:
            core_files.extend([
                f'{self.amslah_path}/core/startup_samd21.c',
                f'{self.amslah_path}/core/mtb.c'
            ])
        else:
            core_files.append(f'{self.amslah_path}/core/startup_samd51.c')
        
        # Add FreeRTOS files
        freertos_files = [
            f'{self.amslah_path}/freertos/croutine.c',
            f'{self.amslah_path}/freertos/event_groups.c',
            f'{self.amslah_path}/freertos/list.c',
            f'{self.amslah_path}/freertos/queue.c',
            f'{self.amslah_path}/freertos/tasks.c',
            f'{self.amslah_path}/freertos/stream_buffer.c',
            f'{self.amslah_path}/freertos/timers.c',
            f'{self.amslah_path}/freertos/FreeRTOS-openocd.c',
            f'{self.amslah_path}/freertos/heap_1.c',
            f'{self.amslah_path}/freertos/portable_samd51/port.c'
        ]
        
        # Add core files (check for conflicts)
        for f in core_files + freertos_files:
            path = Path(f)
            if path.exists():
                basename = path.name
                if basename in all_c_files and all_c_files[basename] != path:
                    raise AssertionError(f"Filename conflict with core file: '{basename}'")
                all_c_files[basename] = path
        
        # Add mutex.cpp
        mutex_path = Path(f'{self.amslah_path}/extra/mutex.cpp')
        if mutex_path.exists():
            if 'mutex.cpp' in all_cpp_files and all_cpp_files['mutex.cpp'] != mutex_path:
                raise AssertionError("Filename conflict with core file: 'mutex.cpp'")
            all_cpp_files['mutex.cpp'] = mutex_path
        
        return {
            'cpp': all_cpp_files,
            'c': all_c_files,
            'h': all_h_files
        }
    
    def copy_sources_to_build(self, sources: Dict[str, Dict[str, Path]]) -> None:
        """Copy all source files to flattened build/src directory"""
        self.src_path.mkdir(parents=True, exist_ok=True)
        
        # Copy C files
        for basename, src_path in sources['c'].items():
            dst_path = self.src_path / basename
            shutil.copy2(src_path, dst_path)
        
        # Copy C++ files
        for basename, src_path in sources['cpp'].items():
            dst_path = self.src_path / basename
            shutil.copy2(src_path, dst_path)
        
        # Copy header files
        for basename, src_path in sources['h'].items():
            dst_path = self.src_path / basename
            shutil.copy2(src_path, dst_path)
        
        print(f"Copied {len(sources['c'])} C files, {len(sources['cpp'])} C++ files, and {len(sources['h'])} header files to {self.src_path}")
    
    def generate_makefile(self, sources: Dict[str, Dict[str, Path]]) -> str:
        """Generate a simple Makefile for make to execute"""
        flags = self.get_mcu_flags()
        includes = self.get_include_paths()
        user_cflags = self.config.get('cflags', [])
        project_name = self.project_path.name
        
        # Add variable flags (like CF) to cflags
        variable_flags = []
        for var_name, var_value in self.variables.items():
            if var_value:  # Only add non-empty values
                variable_flags.append(var_value)
        
        # Convert lists to space-separated strings for Makefile
        cflags_str = ' '.join(flags['cflags'] + user_cflags + variable_flags)
        lflags_str = ' '.join(flags['lflags'])
        
        # Update includes to include build/src since all headers are copied there
        includes_str = ' '.join(includes + [f'-I{self.src_path}'])
        
        # Create object directory
        self.obj_path.mkdir(parents=True, exist_ok=True)
        
        # Generate object file lists (all flattened in build/obj/)
        c_objs = []
        cpp_objs = []
        
        for basename in sources['c'].keys():
            obj_path = self.obj_path / f"{Path(basename).stem}.o"
            c_objs.append(str(obj_path))
            
        for basename in sources['cpp'].keys():
            obj_path = self.obj_path / f"{Path(basename).stem}.o"
            cpp_objs.append(str(obj_path))
        
        # Generate file rules
        file_rules = ""
        
        # Add rules for C files
        for basename in sources['c'].keys():
            src_file = self.src_path / basename
            obj_file = self.obj_path / f"{Path(basename).stem}.o"
            file_rules += f"""
{obj_file}: {src_file}
\t@echo "Compiling {basename}..."
\t$(CC) $(INCLUDES) $(CFLAGS) -o $@ -c $<
"""

        # Add rules for C++ files  
        for basename in sources['cpp'].keys():
            src_file = self.src_path / basename
            obj_file = self.obj_path / f"{Path(basename).stem}.o"
            file_rules += f"""
{obj_file}: {src_file}
\t@echo "Compiling {basename}..."
\t$(CXX) $(INCLUDES) $(CFLAGS) -o $@ -c $<
"""

        # Load template and substitute values
        template_path = self.amslah_path / "makefile.template"
        with open(template_path, 'r') as f:
            template = f.read()
        
        # Substitute placeholders
        makefile_content = template.replace('{{PROJECT_NAME}}', project_name)
        makefile_content = makefile_content.replace('{{BUILD_PATH}}', str(self.build_path))
        makefile_content = makefile_content.replace('{{CFLAGS}}', cflags_str)
        makefile_content = makefile_content.replace('{{LFLAGS}}', lflags_str)
        makefile_content = makefile_content.replace('{{INCLUDES}}', includes_str)
        makefile_content = makefile_content.replace('{{C_OBJS}}', ' '.join(c_objs))
        makefile_content = makefile_content.replace('{{CPP_OBJS}}', ' '.join(cpp_objs))
        makefile_content = makefile_content.replace('{{FILE_RULES}}', file_rules)
        
        return makefile_content
    
    def clean(self):
        """Clean build directory"""
        if self.build_path.exists():
            shutil.rmtree(self.build_path)
            print("Build directory cleaned")
        
        # Also clean zemi_output if it exists
        zemi_output = self.project_path / "zemi_output"
        if zemi_output.exists():
            shutil.rmtree(zemi_output)
            print("Zemi output directory cleaned")
    
    def build(self, jobs: int = 1) -> str:
        """Main build function"""
        print(f"Building project: {self.project_path.name}")
        
        # Create build directory
        self.build_path.mkdir(exist_ok=True)
        
        # Run preprocessing hooks
        if not self.run_hooks():
            sys.exit(1)
        
        # Find source files
        sources = self.find_source_files()
        print(f"Found {len(sources['c'])} C files, {len(sources['cpp'])} C++ files, and {len(sources['h'])} header files")
        
        # Copy all sources to flattened build/src directory
        self.copy_sources_to_build(sources)
        
        # Generate Makefile
        makefile_content = self.generate_makefile(sources)
        with open(self.makefile_path, 'w') as f:
            f.write(makefile_content)
        
        print(f"Generated Makefile: {self.makefile_path}")
        
        # Run make
        make_cmd = ['make', '-f', str(self.makefile_path)]
        if jobs > 1:
            make_cmd.extend(['-j', str(jobs)])
        
        print(f"Running: {' '.join(make_cmd)}")
        try:
            subprocess.run(make_cmd, check=True, cwd=self.project_path)
        except subprocess.CalledProcessError as e:
            print(f"Make failed with exit code {e.returncode}")
            raise e
        
        # Run post hooks
        post_hooks = self.config.get('hooks_post', '')
        if post_hooks:
            # Append binary file path like original Makefile does
            binary_file = f"build/{self.project_path.name}.bin"
            post_hooks_cmd = f"{post_hooks} {binary_file}"
            print(f"Running post hooks: {post_hooks_cmd}")
            subprocess.run(post_hooks_cmd, shell=True, cwd=self.project_path)
        
        binary_path = self.build_path / f"{self.project_path.name}.bin"
        return str(binary_path)

def main():
    parser = argparse.ArgumentParser(description='AMSLAH Build System')
    parser.add_argument('command', nargs='?', default='build', 
                       choices=['build', 'clean'],
                       help='Build command')
    parser.add_argument('-j', '--jobs', type=int, default=1,
                       help='Number of parallel jobs for make')
    parser.add_argument('-c', '--config', default='amslah.yaml',
                       help='Configuration file path')
    
    # Parse known args to handle variable assignments like CF=-DBOARD_REV=1
    args, unknown = parser.parse_known_args()
    
    # Parse variable assignments from unknown args
    variables = {}
    remaining_args = []
    
    for arg in unknown:
        if '=' in arg and not arg.startswith('-'):
            # This looks like a variable assignment
            key, value = arg.split('=', 1)
            variables[key] = value
        else:
            remaining_args.append(arg)
    
    if remaining_args:
        print(f"Warning: Unrecognized arguments: {remaining_args}")
    
    build_system = BuildSystem(args.config, variables)
    
    if args.command == 'clean':
        build_system.clean()
    else:
        try:
            build_system.build(jobs=args.jobs)
        except subprocess.CalledProcessError:
            sys.exit(1)

if __name__ == '__main__':
    main()
