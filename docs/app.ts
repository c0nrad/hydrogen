import * as THREE from 'three';
import { OrbitControls } from 'three/examples/jsm/controls/OrbitControls'

import { wavefunctionData } from './data'
import * as Stats from 'stats.js'

var container;
var camera: THREE.Camera
var scene: THREE.Scene
var renderer: THREE.Renderer
var controls: OrbitControls

var mouseX = 0, mouseY = 0;

var windowHalfX = window.innerWidth / 2;
var windowHalfY = window.innerHeight / 2;

var maxElements = 500;

var dataIndex = 0;

var stats = new Stats();
stats.showPanel(1); // 0: fps, 1: ms, 2: mb, 3+: custom
document.body.appendChild(stats.dom);



init();
animate();

function init() {

    container = document.createElement('div');
    document.body.appendChild(container);

    camera = new THREE.PerspectiveCamera(75, window.innerWidth / window.innerHeight, 1, 10000);
    camera.position.z = 100;

    scene = new THREE.Scene();

    renderData();

    renderer = new THREE.WebGLRenderer({ antialias: true });
    renderer.setSize(window.innerWidth, window.innerHeight);
    container.appendChild(renderer.domElement);

    controls = new OrbitControls(camera, renderer.domElement);
    controls.autoRotateSpeed = 5
    controls.autoRotate = true

    var geometry = new THREE.SphereGeometry(1, 4, 4);
    var material = new THREE.MeshBasicMaterial({ color: "red" });
    var sphere = new THREE.Mesh(geometry, material);
    sphere.userData["p"] = 100000000000
    scene.add(sphere);

    controls.update()
    window.addEventListener('resize', onWindowResize, false);
}

function renderData() {
    while (scene.children.length < maxElements) {
        if (dataIndex >= wavefunctionData.length) {
            dataIndex = 0
        }

        let d = wavefunctionData[dataIndex];
        if (Math.random() < d.p * 10) {
            var geometry = new THREE.SphereGeometry(.5, 4, 4);
            var material = new THREE.MeshBasicMaterial({ color: "blue" });
            var sphere = new THREE.Mesh(geometry, material);
            sphere.position.setFromSphericalCoords(d.r * 5, d.phi, d.theta);
            sphere.userData["p"] = d.p * 10000
            scene.add(sphere);
        }
        dataIndex++
    }
}


function onWindowResize() {

    windowHalfX = window.innerWidth / 2;
    windowHalfY = window.innerHeight / 2;


    renderer.setSize(window.innerWidth, window.innerHeight);
}

//



//

function animate() {
    requestAnimationFrame(animate);

    stats.begin();

    for (let c of scene.children) {
        if (c.userData["p"] <= 0) {
            scene.remove(c)
        } else {
            c.userData["p"]--;
        }
    }

    renderData();

    render();
    stats.end();

}

function render() {
    // camera.position.x += (mouseX - camera.position.x) * .05;
    // camera.position.y += (- mouseY - camera.position.y) * .05;
    // camera.lookAt(scene.position);


    controls.update();
    renderer.render(scene, camera);
}
