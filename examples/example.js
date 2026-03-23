const py = require('./example.py');
const rb = require('./example.rb');
const settings = { count: 1 };

class Runner {
    start() {
        return "started";
    }
}

function run() {
    console.log("Calling Python functions...");
    console.log(py.sum(2, 3));
    console.log(py.mul(4, 5));

    console.log("Calling Ruby function...");
    console.log(rb.greet());
}

run();