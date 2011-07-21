#!/usr/bin/env ruby

# Requires Ruby 1.9 and "gnuplot" gem

require 'gnuplot'

X0 = 0.0
XN = 0.4
Y0 = 2.0
N = 100

# the differential equation definition
# f(x, y) = dy / dx
def f(x, y); (1.0 + y * y) / (1.0 + x * x); end

# the analytic solution of the differential equation
def y(x); Math.tan(Math.atan(x) + Math.atan(2)); end


class Approximator
  attr_reader :x, :y

  def initialize(x0, y0, h, &f)
    @x = x0.to_f
    @y = y0.to_f
    @h = h.to_f
    @f = f
  end
end

class Euler < Approximator
  def next
    return @y0 = @y unless @y0
    @y += @h * @f.call(@x, @y)
    @x += @h
    @y
  end
end

class RungeKutta < Approximator
  def next
    return @y0 = @y unless @y0
    k1 = @h * @f.call(@x, @y)
    k2 = @h * @f.call(@x + @h / 2, @y + k1 / 2)
    k3 = @h * @f.call(@x + @h / 2, @y + k2 / 2)
    k4 = @h * @f.call(@x + @h, @y + k3)
    @y += (k1 + k4 + 2 * (k2 + k3)) / 6
    @x += @h
    @y
  end
end



Gnuplot.open do |gp|
  Gnuplot::Plot.new(gp) do |plot|
    plot.title  'Approximation of Differential Equations'
    plot.ylabel 'log E'
    plot.xlabel 'log h'
    plot.key 'bottom right'
    plot.size 'ratio 1'
    plot.logscale 'xy'

    euler_error = []
    runge_kutta_error = []
    hs = []

    (1..3).step(0.1) do |exponent|
      h = 10 ** (-exponent)

      euler = Euler.new(X0, Y0, h) { |x, y| f(x, y) }
      runge_kutta = RungeKutta.new(X0, Y0, h) { |x, y| f(x, y) }

      # written for clarity as two loops, but
      # one could be used, since the two progress at the same speed
      while runge_kutta.x < XN
        runge_kutta.next
      end
      while euler.x < XN
        euler.next
      end

      hs << h
      euler_error << (y(euler.x) - euler.y).abs
      runge_kutta_error << (y(runge_kutta.x) - runge_kutta.y).abs
    end

    plot.data << Gnuplot::DataSet.new([hs, euler_error]) do |ds|
      ds.with = "linespoints"
      ds.title = "Euler Method"
    end

    plot.data << Gnuplot::DataSet.new([hs, runge_kutta_error]) do |ds|
      ds.with = "linespoints"
      ds.title = "Runge-Kutta Method"
    end
  end
end
