#!/usr/bin/env ruby

# Requires Ruby 1.9 and "gnuplot" gem

require 'gnuplot'

A = 0.0
B = Math::PI / 2
MAX_N = 20000

def f(x); x * Math.cos(x); end
analytic_integral = (Math::PI - 2) / 2


Gnuplot.open do |gp|
  Gnuplot::Plot.new(gp) do |plot|
    plot.title  'Numerical Integration Error'
    plot.ylabel 'log E'
    plot.xlabel 'log h'
    plot.key 'top left'
    plot.size 'ratio 1'
    plot.logscale 'xy'

    n = 3
    h = []
    trapezoid = []
    simpson = []

    while n < MAX_N
      n *= 2
      current_h = (B - A) / n
      h << current_h
      x = []
      f = []
      (A..B).step(current_h) do |xi|
        x << xi
        f << f(xi)
      end

      even = false
      even_f, odd_f = f.partition { even = !even }

      trapezoid <<
          current_h / 2 *
            (f[0] + f[-1] +                 # first and last
             2 * f[1..-2].inject(:+))       # sum of all in between
      simpson <<
          current_h / 3 *
            (f[0] + f[-1] +                 # first and last
             2 * even_f[1..-2].inject(:+) + # sum of even ones between them
             4 * odd_f.inject(0, :+))       # sum of odd ones between them
    end
    
    trapezoid_error = trapezoid.map { |trapezoid_integral|
        (analytic_integral - trapezoid_integral).abs }
    simpson_error = simpson.map { |simpson_integral|
        (analytic_integral - simpson_integral).abs }
    
    plot.data << Gnuplot::DataSet.new([h, trapezoid_error]) do |ds|
      ds.with = "linespoints"
      ds.title = "Trapezoid rule"
    end

    plot.data << Gnuplot::DataSet.new([h, simpson_error]) do |ds|
      ds.with = "linespoints"
      ds.title = "Simpson rule"
    end
  end
end
