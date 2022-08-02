def build_antd()
{
  sh '''
  set -e
  cd $WORKSPACE
  mkdir -p build/$arch/opt/www
  [ -f Makefile ] && make clean
  libtoolize
  aclocal
  autoconf
  automake --add-missing
  ./configure --prefix=/opt/www
  CFLAGS="-I$WORKSPACE../ant-http/build/$arch/usr/include" LDFLAGS="-L$WORKSPACE../ant-http/build/$arch/usr/lib" make
  DESTDIR=$WORKSPACE/build/$arch make install
  '''
}

pipeline{
  agent { node{ label'master' }}
  options {
    // Limit build history with buildDiscarder option:
    // daysToKeepStr: history is only kept up to this many days.
    // numToKeepStr: only this many build logs are kept.
    // artifactDaysToKeepStr: artifacts are only kept up to this many days.
    // artifactNumToKeepStr: only this many builds have their artifacts kept.
    buildDiscarder(logRotator(numToKeepStr: "1"))
    // Enable timestamps in build log console
    timestamps()
    // Maximum time to run the whole pipeline before canceling it
    timeout(time: 1, unit: 'HOURS')
    // Use Jenkins ANSI Color Plugin for log console
    ansiColor('xterm')
    // Limit build concurrency to 1 per branch
    disableConcurrentBuilds()
  }
  stages
  {
    stage('Build AMD64') {
      agent {
          docker {
              image 'xsangle/ci-tools:latest-amd64'
              // Run the container on the node specified at the
              // top-level of the Pipeline, in the same workspace,
              // rather than on a new node entirely:
              reuseNode true
              registryUrl 'http://workstation:5000/'
          }
      }
      steps {
        script{
          env.arch = "amd64"
        }
        build_plugin()
      }
    }
    stage('Build ARM64') {
      agent {
          docker {
              image 'xsangle/ci-tools:latest-arm64'
              // Run the container on the node specified at the
              // top-level of the Pipeline, in the same workspace,
              // rather than on a new node entirely:
              reuseNode true
              registryUrl 'http://workstation:5000/'
          }
      }
      steps {
        script{
          env.arch = "arm64"
        }
        build_plugin()
      }
    }
    stage('Build ARM') {
      agent {
          docker {
              image 'xsangle/ci-tools:latest-arm'
              // Run the container on the node specified at the
              // top-level of the Pipeline, in the same workspace,
              // rather than on a new node entirely:
              reuseNode true
              registryUrl 'http://workstation:5000/'
          }
      }
      steps {
        script{
          env.arch = "arm"
        }
        build_plugin()
      }
    }
    stage('Archive') {
      steps {
        script {
            archiveArtifacts artifacts: 'build/', fingerprint: true
        }
      }
    }
  }
}
