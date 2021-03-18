apply plugin: 'java'
apply plugin: 'com.google.protobuf'

buildscript {
    repositories {
        maven { // The google mirror is less flaky than mavenCentral()
            url "https://maven-central.storage-download.googleapis.com/repos/central/data/" }
    }
    dependencies { // ASSUMES GRADLE 2.12 OR HIGHER. Use plugin version 0.7.5 with earlier
        // gradle versions
        classpath 'com.google.protobuf:protobuf-gradle-plugin:0.8.10' }
}

repositories {
    maven { // The google mirror is less flaky than mavenCentral()
        url "https://maven-central.storage-download.googleapis.com/repos/central/data/" }
    mavenLocal()
}

// IMPORTANT: You probably want the non-SNAPSHOT version of gRPC. Make sure you
// are looking at a tagged version of the example and not "master"!

// Feel free to delete the comment at the next line. It is just for safely
// updating the version in our release process.
def grpcVersion = "@CURRENT_GRPC_VERSION@"
def protobufVersion = "@CURRENT_PROTOBUF_VERSION@"

dependencies {
    compile "com.google.api.grpc:proto-google-common-protos:1.0.0"
    compile "io.grpc:grpc-alts:@CURRENT_GRPC_VERSION@"
    compile "io.grpc:grpc-protobuf:@CURRENT_GRPC_VERSION@"
    compile "io.grpc:grpc-stub:@CURRENT_GRPC_VERSION@"
    compileOnly "javax.annotation:javax.annotation-api:1.2"
}

sourceSets {
  main {
    proto {
      // In addition to the default 'src/main/proto'
      srcDir '../Protos'
    }
  }
}

protobuf {
  protoc {
    artifact = "com.google.protobuf:protoc:@CURRENT_PROTOBUF_VERSION@"
  }
  plugins {
    grpc {
      artifact = "io.grpc:protoc-gen-grpc-java:@CURRENT_GRPC_VERSION@"
    }
  }
  generateProtoTasks {
    all()*.plugins {
      grpc {}
    }
  }
}

jar {
  manifest {
    attributes 'Main-Class': 'org.caffa.rpc.AppTest'
  }
  from {
    configurations.compile.collect { it.isDirectory() ? it : zipTree(it) }
    configurations.runtimeClasspath.collect { it.isDirectory() ? it : zipTree(it) }
  }
}
