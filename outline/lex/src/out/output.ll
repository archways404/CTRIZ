; ModuleID = 'MyModule'
source_filename = "MyModule"

@.str_int = private constant [4 x i8] c"%d\0A\00"
@.str_float = private constant [4 x i8] c"%f\0A\00"

define void @main() {
entry:
  %y = alloca float, align 4
  %x = alloca i32, align 4
  store i32 5, ptr %x, align 4
  store float 1.050000e+01, ptr %y, align 4
  store i32 3, ptr %x, align 4
  %x1 = load i32, ptr %x, align 4
  %0 = call i32 (ptr, ...) @printf(ptr @.str_int, i32 %x1)
  %y2 = load float, ptr %y, align 4
  %fpext = fpext float %y2 to double
  %1 = call i32 (ptr, ...) @printf(ptr @.str_float, double %fpext)
  ret void
}

declare i32 @printf(ptr, ...)
