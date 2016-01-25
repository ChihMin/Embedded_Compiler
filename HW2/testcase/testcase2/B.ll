; ModuleID = 'B.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @func() #0 {
entry:
  %a = alloca i32, align 4
  %b = alloca i32, align 4
  %c = alloca i32, align 4
  %d = alloca i32, align 4
  %e = alloca i32, align 4
  %f = alloca i32, align 4
  %g = alloca i32, align 4
  %h = alloca i32, align 4
  %i = alloca i32, align 4
  %j = alloca i32, align 4
  store i32 10, i32* %a, align 4
  store i32 100, i32* %b, align 4
  %0 = load i32, i32* %a, align 4
  %1 = load i32, i32* %b, align 4
  %add = add nsw i32 %0, %1
  store i32 %add, i32* %c, align 4
  %2 = load i32, i32* %a, align 4
  %sub = sub nsw i32 %2, 2
  store i32 %sub, i32* %d, align 4
  %3 = load i32, i32* %a, align 4
  %4 = load i32, i32* %b, align 4
  %add1 = add nsw i32 %3, %4
  store i32 %add1, i32* %b, align 4
  %5 = load i32, i32* %a, align 4
  %cmp = icmp eq i32 %5, 0
  br i1 %cmp, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  %6 = load i32, i32* %b, align 4
  %7 = load i32, i32* %a, align 4
  %add2 = add nsw i32 %6, %7
  store i32 %add2, i32* %e, align 4
  %8 = load i32, i32* %a, align 4
  %add3 = add nsw i32 %8, 2
  store i32 %add3, i32* %f, align 4
  br label %if.end

if.else:                                          ; preds = %entry
  %9 = load i32, i32* %a, align 4
  %add4 = add nsw i32 %9, 2
  store i32 %add4, i32* %g, align 4
  %10 = load i32, i32* %a, align 4
  %mul = mul nsw i32 %10, 2
  store i32 %mul, i32* %h, align 4
  %11 = load i32, i32* %b, align 4
  %div = sdiv i32 %11, 3
  store i32 %div, i32* %i, align 4
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  store i32 2, i32* %d, align 4
  %12 = load i32, i32* %h, align 4
  %mul5 = mul nsw i32 %12, 3
  store i32 %mul5, i32* %j, align 4
  store i32 123, i32* %a, align 4
  %13 = load i32, i32* %a, align 4
  %14 = load i32, i32* %b, align 4
  %add6 = add nsw i32 %13, %14
  store i32 %add6, i32* %a, align 4
  %15 = load i32, i32* %j, align 4
  %cmp7 = icmp eq i32 %15, 10
  br i1 %cmp7, label %if.then.8, label %if.end.10

if.then.8:                                        ; preds = %if.end
  %16 = load i32, i32* %h, align 4
  %mul9 = mul nsw i32 %16, 3
  store i32 %mul9, i32* %j, align 4
  br label %if.end.10

if.end.10:                                        ; preds = %if.then.8, %if.end
  %17 = load i32, i32* %h, align 4
  %mul11 = mul nsw i32 %17, 3
  store i32 %mul11, i32* %e, align 4
  ret void
}

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.7.0 (https://github.com/llvm-mirror/clang e2c9b2285d808b1ac25825f973a8f591c5ddd58d) (https://github.com/llvm-mirror/llvm.git ce75c809a805fa97e836a4cdf6999ac4584e5ad4)"}
