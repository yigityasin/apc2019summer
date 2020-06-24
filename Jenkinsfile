pipeline {
  agent any
  stages {
    stage('Build') {
      steps {
        dir(path: 'exercise 3') {
          sh 'ls'
          sh 'gcc -o increment increment.c -lpthread'
        }

      }
    }

    stage('Deploy') {
      steps {
        dir(path: 'exercise 3') {
          archiveArtifacts 'increment'
        }

      }
    }

  }
}