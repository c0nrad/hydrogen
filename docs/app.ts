import * as THREE from 'three';
import { OrbitControls } from 'three/examples/jsm/controls/OrbitControls'

import { wavefunctionData } from './data'
import * as Stats from 'stats.js'

interface Wavefunction {
    n: number
    l: number
    m: number

    data: any
}

var container;
var camera: THREE.Camera
var scene: THREE.Scene
var renderer: THREE.Renderer
var controls: OrbitControls

//http://workshop.chromeexperiments.com/examples/gui/#1--Basic-Usage

var maxElements = 50;
var dataIndex = 0;
var minimumProbability = 0.00001
// var minimumProbability = 0.000001

var stats = new Stats();
stats.showPanel(1); // 0: fps, 1: ms, 2: mb, 3+: custom
document.body.appendChild(stats.dom);

function getColor(n, l, m: number): string {
    let out = ""
    switch (n) {
        case 2:
            out += "#00"
            break
        case 3:
            out += "#8B"
            break
        case 4:
            out += "#FF"
            break
    }

    switch (l) {
        case 0:
            out += "00"
            break
        case 1:
            out += "8B"
            break
        case 2:
            out += "FF"
            break
    }

    switch (m) {
        case 0:
            out += "00"
            break
        case 1:
            out += "8B"
            break
        case 2:
            out += "FF"
            break
    }

    return out
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

    // renderData();
    initData()
    // var vertices = [];

    // for (var i = 0; i < 10000; i++) {

    //     var x = THREE.MathUtils.randFloatSpread(2000);
    //     var y = THREE.MathUtils.randFloatSpread(2000);
    //     var z = THREE.MathUtils.randFloatSpread(2000);

    //     vertices.push(x, y, z);

    // }

    // var geometry = new THREE.BufferGeometry();
    // geometry.setAttribute('position', new THREE.Float32BufferAttribute(vertices, 3));

    // var material = new THREE.PointsMaterial({ color: 0x888888 });

    // var points = new THREE.Points(geometry, material);

    // scene.add(points);


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
    for (let wavefunction of wavefunctionData) {
        console.log(wavefunction.n, wavefunction.l, wavefunction.m, wavefunction.count)
        while (wavefunction.count < maxElements) {
            if (dataIndex >= wavefunction.data.length) {
                dataIndex = 0
            }

            let d = wavefunction.data[dataIndex];
            if (Math.random() < d.p * 200) {
                var geometry = new THREE.SphereGeometry(.5, 4, 4);
                var material = new THREE.MeshBasicMaterial({ color: getColor(wavefunction.n, wavefunction.l, wavefunction.m) });
                var sphere = new THREE.Mesh(geometry, material);
                sphere.position.setFromSphericalCoords(d.r * 3, d.phi, d.theta);
                sphere.userData["p"] = d.p * 100000
                sphere.userData["n"] = wavefunction.n
                sphere.userData["l"] = wavefunction.l
                sphere.userData["m"] = wavefunction.m
                scene.add(sphere);

                wavefunction.count++
            }
            dataIndex++
        }
    }
}

function initData() {
    var vertices = [];
    var alphas = [];

    for (let wavefunction of wavefunctionData) {
        console.log(wavefunction.n, wavefunction.l, wavefunction.m, wavefunction.count)

        for (let d of wavefunction.data) {
            if (d.p <= minimumProbability) {
                continue
            }

            var v = new THREE.Vector3(0, 0, 0);
            v.setFromSphericalCoords(d.r * 5, d.phi, d.theta);
            vertices.push(v.x, v.y, v.z);
            console.log(d.r, d.phi, d.theta, d.p, "calulated", v.x, v.y, v.z)


            alphas.push(getAlpha(d.p))
        }

        var geometry = new THREE.BufferGeometry();
        geometry.setAttribute('position', new THREE.Float32BufferAttribute(vertices, 3));
        geometry.addAttribute('alpha', new THREE.Float32BufferAttribute(alphas, 1));

        // uniforms
        let uniforms = {
            color: { value: new THREE.Color(0xffff00) },
        };

        // point cloud material
        var shaderMaterial = new THREE.ShaderMaterial({
            uniforms: uniforms,
            vertexShader: document.getElementById('vertexshader').textContent,
            fragmentShader: document.getElementById('fragmentshader').textContent,
            transparent: true
        });

        var points = new THREE.Points(geometry, shaderMaterial);

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
