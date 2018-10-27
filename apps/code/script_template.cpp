#include "script_template.h"

namespace Code {

constexpr ScriptTemplate emptyScriptTemplate(".py", "\x01" R"(from math import *
)");

constexpr ScriptTemplate factorialScriptTemplate("factorial.py", "\x01" R"(def factorial(n):
  if n == 0:
    return 1
  else:
    return n * factorial(n-1))");

constexpr ScriptTemplate fibonacciScriptTemplate("fibonacci.py", "\x01" R"(def fibo(n):
  a=0
  b=1
  for i in range(1,n+1):
    c=a+b
    a=b
    b=c
  return a

def fibo2(n):
  if n==0:
    return 0
  elif n==1 or n==2:
    return 1
  return fibo2(n-1)+fibo2(n-2))");

constexpr ScriptTemplate mandelbrotScriptTemplate("mandelbrot.py", "\x01" R"(# This script draws a Mandelbrot fractal set
# N_iteration: degree of precision
import kandinsky
def mandelbrot(N_iteration):
  for x in range(320):
    for y in range(222):
# Compute the mandelbrot sequence for the point c = (c_r, c_i) with start value z = (z_r, z_i)
      z = complex(0,0)
# Rescale to fit the drawing screen 320x222
      c = complex(3.5*x/319-2.5, -2.5*y/221+1.25)
      i = 0
      while (i < N_iteration) and abs(z) < 2:
        i = i + 1
        z = z*z+c
# Choose the color of the dot from the Mandelbrot sequence
      rgb = int(255*i/N_iteration)
      col = kandinsky.color(int(rgb),int(rgb*0.75),int(rgb*0.25))
# Draw a pixel colored in 'col' at position (x,y)
      kandinsky.set_pixel(x,y,col))");

constexpr ScriptTemplate polynomialScriptTemplate("polynomial.py", "\x01" R"(from math import *
# roots(a,b,c) computes the solutions of the equation a*x**2+b*x+c=0
def roots(a,b,c):
  delta = b*b-4*a*c
  if delta == 0:
    return -b/(2*a)
  elif delta > 0:
    x_1 = (-b-sqrt(delta))/(2*a)
    x_2 = (-b+sqrt(delta))/(2*a)
    return x_1, x_2
  else:
    return None)");

constexpr ScriptTemplate spiralScriptTemplate("spiral.py", "\x00" R"(from turtle import *
for i in range(255):
  gray=255-i
  color(gray,int(gray*0.75),int(gray*0.25))
  forward(i*0.1)
  left(10))");

const ScriptTemplate * ScriptTemplate::Empty() {
  return &emptyScriptTemplate;
}

const ScriptTemplate * ScriptTemplate::Factorial() {
  return &factorialScriptTemplate;
}

const ScriptTemplate * ScriptTemplate::Fibonacci() {
  return &fibonacciScriptTemplate;
}

const ScriptTemplate * ScriptTemplate::Mandelbrot() {
  return &mandelbrotScriptTemplate;
}

const ScriptTemplate * ScriptTemplate::Polynomial() {
  return &polynomialScriptTemplate;
}

const ScriptTemplate * ScriptTemplate::Spiral() {
  return &spiralScriptTemplate;
}

}


