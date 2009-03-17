// Benchmark tests for Google Protocol Buffers
//
// $Id$

#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

#include "google_size.pb.h"
#include "google_speed.pb.h"

#include <iostream>
#include <fstream>
#include <string>

#include <boost/bind.hpp>
#include <boost/function.hpp>

static const double MIN_SAMPLE_TIME = 1.0;
static const double TARGET_TIME     = 5.0;

using namespace std;
using namespace google::protobuf;
using namespace google::protobuf::io;

double
timeAction (const boost::function<void()>& func, size_t iterations)
{
    clock_t begin = clock ();

    while (iterations--) {
        func ();
    }

    double elapsed = static_cast<double> (clock () - begin) / CLOCKS_PER_SEC;
    
    return elapsed;
}

void
benchmark (const char* name, size_t dataSize, const boost::function<void()>& func)
{
    size_t iterations = 1;
    double elapsed = timeAction (func, iterations);

    while (elapsed < MIN_SAMPLE_TIME) {
        iterations <<= 1;
        elapsed = timeAction (func, iterations);
    }

    iterations = (TARGET_TIME / elapsed) * iterations;

    elapsed = timeAction (func, iterations);

    cout << name << ": " << iterations << " ops in " << elapsed << "s; "
         << iterations / elapsed << " ops/sec; "
         << 1.0e6 * elapsed / iterations << " usec/op; "
         << (iterations * dataSize) / (elapsed * 1048576.0)
         << " MiB/s"
         << endl;
}

bool
runTest (const char* type, const char* filename)
{
    cout << "Benchmarking " << type << " with file " << filename << endl;

    const Descriptor* desc = DescriptorPool::generated_pool()->
        FindMessageTypeByName (type);

    if (!desc) {
        cerr << "Can't find Descriptor for " << type << endl;
        return false;
    }

    Message* msg = MessageFactory::generated_factory()->GetPrototype(desc)->New();

    if (!msg) {
        cerr << "GetPrototype for " << type << " failed" << endl;
        return false;
    }

    ifstream input (filename);
    IstreamInputStream istr (&input);

    if (!msg->ParseFromZeroCopyStream (&istr)) {
        cerr << "Failed to parse " << type << " from " << filename << endl;
        return false;
    }

    std::string wire_form = msg->SerializeAsString ();

    benchmark ("SerializeAsString", wire_form.size(),
               boost::bind (&Message::SerializeAsString, msg));

    benchmark ("SerializeToString", wire_form.size(),
               boost::bind (&Message::SerializeToString, msg, &wire_form));

    std::vector<char> buf (wire_form.size(), 0);

    benchmark ("SerializeToArray", wire_form.size(),
               boost::bind (&Message::SerializeToArray, msg,
                            &buf[0], wire_form.size()));

    msg->Clear ();

    benchmark ("ParseFromString", wire_form.size (),
               boost::bind (&Message::ParseFromString, msg,
                            boost::ref (wire_form)));

    benchmark ("ParseFromArray", wire_form.size (),
               boost::bind (&Message::ParseFromArray, msg,
                            wire_form.data(), wire_form.size()));

    return true;
}

int
main (int argc, char** argv)
{
    if (argc < 3 || argc % 2 == 0) {
        cerr << "Usage: " << argv[0] << " type filename\n";
        return 1;
    }

    for (int i = 1; i < argc; i += 2) {
        runTest (argv[i], argv[i + 1]);
    }

    return 0;
}
