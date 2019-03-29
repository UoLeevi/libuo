node {
    stage('Preparation') {
        git 'https://github.com/UoLeevi/libuo.git'
    }
    stage('Build') {
        sh 'sudo scripts/rebuild.sh'
    }
    stage('Test') {
        sh 'scripts/test.sh'
    }
}
