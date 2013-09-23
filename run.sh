./logic_simulate configure.arl > arl.output
./logic_simulate configure.celebrity > celebrity.output
./logic_simulate configure.5000random > 5000random.output
diff arl.output golden.arl.output
diff celebrity.output golden.celebrity.output
diff 5000random.output golden.5000random.output
