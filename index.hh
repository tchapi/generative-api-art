<?hh

require "config.hh";
require "GenerativeApiArt.hh";

$test = new GenerativeApiArt(Config::KEY, 500);
$test->call()->extract()->create(GenerativeApiArt::MODE_BITMAP);

$test->output();