#!/usr/bin/env ruby

DELTA   = 0.00001   # used for numeric derivation
EPSILON = 0.0000001 # used for solution precision


# functions
f = lambda { |x| x**3 - 6 * x**2 + 11 * x - 6 }
g = lambda { |x| x**2 - Math.cos(x) }

# numerical derivatives
def deriv(f)
  lambda { |x| (f.call(x + DELTA) - f.call(x - DELTA)) / (2 * DELTA) }
end

ndf = deriv(f)
ndg = deriv(g)

# analytic derivatives
adf = lambda { |x| 3 * x**2 - 12 * x + 11 }
adg = lambda { |x| 2 * x + Math.sin(x) }

def bisect(f, a, b)
  table = [a, b]
  begin
    fa = f.call(a)
    fb = f.call(b)
    c = (a + b) / 2.0
    table.push(c)
    fc = f.call(c)
    if (fc * fa) < 0
      b = c
    else
      a = c
    end
  end while (b - a >= EPSILON)
  table
end

def newton(f, fd, a)
  table = [a]
  begin
    fa = f.call(a)
    c = a - f.call(a) / fd.call(a).to_f
    table.push(c)
    a, b = c, a
  end while ((a - b).abs >= EPSILON)
  table
end

puts bisect(f, 0, 1.5).inspect
puts newton(f, ndf, 0).inspect
puts newton(f, adf, 0).inspect
puts "----"
puts bisect(f, 1.5, 2.3).inspect # WEIRD!
puts newton(f, ndf, 2.3).inspect
puts newton(f, adf, 2.3).inspect
puts "----"
puts bisect(f, 2.3, 10).inspect
puts newton(f, ndf, 10).inspect
puts newton(f, adf, 10).inspect
puts "----"
puts bisect(g, -2, 0).inspect
puts newton(g, ndg, -2).inspect
puts newton(g, adg, -2).inspect
puts "----"
puts bisect(g, 0, 10).inspect
puts newton(g, ndg, 10).inspect
puts newton(g, adg, 10).inspect
