import * as THREE from 'three';
import { OrbitControls } from 'three/examples/jsm/controls/OrbitControls'

import { wavefunctionData } from './data'
import * as Stats from 'stats.js'

import * as dat from 'dat.gui';

interface Wavefunction {
    n: number
    l: number
    m: number

    data: any
}


var state = {
    minProb: .00001,
    wavefunction: "322",
}

var gui = new dat.GUI();
function initGui() {

    gui.add(state, "minProb").onChange(() => {
        clearData();
        initData();
    })

    let wavefunctions = []
    for (let w of wavefunctionData) {
        wavefunctions.push(mergeQuantumNumber(w.n, w.l, w.m))
    }

    // Choose from accepted values
    gui.add(state, 'wavefunction', wavefunctions).onChange(() => {
        clearData();
        initData();
    })

}


var container;
var camera: THREE.Camera
var scene: THREE.Scene
var renderer: THREE.Renderer
var controls: OrbitControls

//http://workshop.chromeexperiments.com/examples/gui/#1--Basic-Usage

// var minimumProbability = 0.000001

var stats = new Stats();
stats.showPanel(1); // 0: fps, 1: ms, 2: mb, 3+: custom
document.body.appendChild(stats.dom);

function mergeQuantumNumber(n, l, m): string {
    return "" + n + l + m;
}

function getColor(n, l, m: number): string {
    var colors = ["#0275d8", "#5cb85c", "#5bc0de", "#f0ad4e", "#d9534f", "#292b2c", "#f7f7f7"]

    let q = mergeQuantumNumber(n, l, m)
    switch (q) {
        case "200":
            return colors[0];
        case "210":
            return colors[1];
        case "211":
            return colors[2];
        case "300":
            return colors[3];
        case "310":
            return colors[4];
        case "320":
            return colors[5];
    }

    return "white"
}

function getAlpha(p: number): number {
    if (p > .1) {
        return 1
    }
    if (p > .01) {
        return .8
    }

    if (p > .001) {
        return .6
    }
    if (p > .0001) {
        return .4
    }
    if (p > .00001) {
        return .2
    }
    return .1
}

init();
animate();

function init() {

    container = document.createElement('div');
    document.body.appendChild(container);

    camera = new THREE.PerspectiveCamera(75, window.innerWidth / window.innerHeight, 1, 10000);
    camera.position.z = 100;

    scene = new THREE.Scene();

    initGui()
    initData()

    renderer = new THREE.WebGLRenderer({ antialias: true });
    renderer.setSize(window.innerWidth, window.innerHeight);
    container.appendChild(renderer.domElement);

    controls = new OrbitControls(camera, renderer.domElement);
    controls.autoRotateSpeed = 5
    controls.autoRotate = true

    controls.update()
    window.addEventListener('resize', onWindowResize, false);
}

function clearData() {
    while (scene.children.length != 0) {
        scene.remove(scene.children[0])
    }
}

function initData() {
    console.log(state)

    var geometry = new THREE.SphereGeometry(.5, 4, 4);
    var material = new THREE.MeshBasicMaterial({ color: "red" });
    var sphere = new THREE.Mesh(geometry, material);
    scene.add(sphere);


    for (let wavefunction of wavefunctionData) {
        console.log(wavefunction.n, wavefunction.l, wavefunction.m)

        if (state.wavefunction[0] != wavefunction.n.toString() ||
            state.wavefunction[1] != wavefunction.l.toString() ||
            state.wavefunction[2] != wavefunction.m.toString()) {
            continue
        }
        console.log("MATCH")

        let vertices = [];
        let alphas = [];

        for (let d of wavefunction.data) {
            if ((d.p) <= state.minProb) {
                continue
            }

            let v = new THREE.Vector3(0, 0, 0);
            v.setFromSphericalCoords(d.r * 5, d.phi, d.theta);
            vertices.push(v.x, v.y, v.z);

            alphas.push(getAlpha(d.p))
        }

        let geometry = new THREE.BufferGeometry();
        geometry.setAttribute('position', new THREE.Float32BufferAttribute(vertices, 3));
        geometry.addAttribute('alpha', new THREE.Float32BufferAttribute(alphas, 1));

        // uniforms
        let uniforms = {
            color: { value: new THREE.Color(getColor(wavefunction.n, wavefunction.l, wavefunction.m)) },
        };

        // point cloud material
        let shaderMaterial = new THREE.ShaderMaterial({
            uniforms: uniforms,
            vertexShader: document.getElementById('vertexshader').textContent,
            fragmentShader: document.getElementById('fragmentshader').textContent,
            transparent: true
        });

        let points = new THREE.Points(geometry, shaderMaterial);

        scene.add(points);
    }
}

function onWindowResize() {

    renderer.setSize(window.innerWidth, window.innerHeight);
}

function animate() {
    requestAnimationFrame(animate);

    stats.begin();

    // for (let c of scene.children) {
    //     if (c.userData["p"] <= 0) {
    //         scene.remove(c)

    //         for (let wavefunction of wavefunctionData) {
    //             if (wavefunction.n == c.userData["n"] && wavefunction.l == c.userData["l"] && wavefunction.m == c.userData["m"]) {
    //                 wavefunction.count--;
    //             }
    //         }

    //     } else {
    //         c.userData["p"]--;
    //     }
    // }

    // renderData();

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
