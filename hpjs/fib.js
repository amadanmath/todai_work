function fib(n) {
  if (n < 2) return 1;
  return fib(n - 1) + fib(n - 2);
}
self.onmessage = function(evt) {
  fib(parseInt(evt.data, 10))
  self.postMessage("138");
};
