#!/usr/bin/env ruby

require 'erb'

DELTA   = 0.00001   # used for numeric derivation
EPSILON = 0.000005  # used for solution precision


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
  end while (b - a >= EPSILON && fc != 0)
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

bisects = []
nnewton = []
anewton = []

bisects << bisect(f, 0, 1.5)
nnewton << newton(f, ndf, 0)
anewton << newton(f, adf, 0)

bisects << bisect(f, 1.5, 2.3)
nnewton << newton(f, ndf, 2.3)
anewton << newton(f, adf, 2.3)

bisects << bisect(f, 2.3, 10)
nnewton << newton(f, ndf, 10)
anewton << newton(f, adf, 10)

bisects << bisect(g, -2, 0)
nnewton << newton(g, ndg, -2)
anewton << newton(g, adg, -2)

bisects << bisect(g, 0, 10)
nnewton << newton(g, ndg, 10)
anewton << newton(g, adg, 10)

# output
$table_template, $page_template = DATA.read.split("____\n")

# Array#transpose can only transpose square arrays
def transpose(array)
  tarray = []
  array.each_with_index do |row, rowindex|
    row.each_with_index do |datum, colindex|
      tarray[colindex] ||= []
      tarray[colindex][rowindex] = datum
    end
  end
  tarray
end

def tabulate(data, title, inc)
  tdata = transpose(data)
  erb = ERB.new($table_template)
  erb.result(binding)
end

tables = []
tables << tabulate(bisects, 'Bisection', 1)
tables << tabulate(nnewton, 'Newton-Raphson with a numeric derivative', 0)
tables << tabulate(anewton, 'Newton-Raphson with an analytic derivative', 0)

erb = ERB.new($page_template)
erb.run(binding)

__END__
<table>
  <caption><%= title %></caption>
  <tr>
    <th></th>
    <th colspan="3">
      <i>x</i><sup>3</sup> - 6<i>x</i><sup>2</sup> + 11<i>x</i> - 6
    </th>
    <th colspan="2">
      <i>x</i><sup>2</sup> - cos <i>x</i>
    </th>
  </tr>
  <tr class="var">
    <th>#</th>
    <th>x<sub>1</th>
    <th>x<sub>2</th>
    <th>x<sub>3</th>
    <th>x<sub>1</th>
    <th>x<sub>2</th>
  </tr>
<%
  tdata.each_with_index do |row, rowindex|
%>  <tr>
  <td><%= if rowindex > inc then rowindex - inc else 0 end %></td>
<%
    row.each do |datum|
%>    <td><%= '%2.12f' % datum if datum %></td>
<%
    end
%>  </tr>
<%
  end
%></table>
____
<!doctype html>
<html>
  <head>
    <title>Numeric approximations of real function roots</title>
    <style>
      table {
        width: 100%;
        border: 1px solid black;
        page-break-after: always;
      }
      table td, table th {
        text-align: right;
      }
    </style>
  </head>
  <body>
<%
  tables.each do |table|
%><%= table %>
<%
  end
%>
  </body>
</html>
