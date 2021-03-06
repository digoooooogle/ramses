//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-client.h"

#include <thread>

/**
 * @example ramses-example-basic-rendergroups/src/main.cpp
 * @brief Basic Render Groups Example
 */

int main(int argc, char* argv[])
{
    // register at RAMSES daemon
    ramses::RamsesFramework framework(argc, argv);
    ramses::RamsesClient ramses("ramses-example-basic-rendergroups", framework);
    framework.connect();

    // prepare triangle geometry: vertex position array and index array
    float vertexPositionsArray[] = { -0.5f, 0.f, -1.f, 0.5f, 0.f, -1.f, -0.5f, 1.f, -1.f, 0.5f, 1.f, -1.f };
    const ramses::Vector3fArray* vertexPositions = ramses.createConstVector3fArray(4, vertexPositionsArray);

    uint16_t indicesArray[] = { 0, 1, 2, 2, 1, 3 };
    const ramses::UInt16Array* indices = ramses.createConstUInt16Array(6, indicesArray);

    ramses::EffectDescription effectDesc;
    effectDesc.setVertexShaderFromFile("res/ramses-example-basic-rendergroups.vert");
    effectDesc.setFragmentShaderFromFile("res/ramses-example-basic-rendergroups.frag");
    effectDesc.setUniformSemantic("mvpMatrix", ramses::EEffectUniformSemantic_ModelViewProjectionMatrix);
    const ramses::Effect* effectTex = ramses.createEffect(effectDesc, ramses::ResourceCacheFlag_DoNotCache, "glsl shader");

    /// [Basic Render Groups Example]
    // IMPORTANT NOTE: For simplicity and readability the example code does not check return values from API calls.
    //                 This should not be the case for real applications.

    // create a scene using explicit render passes
    ramses::Scene* scene = ramses.createScene(123u, ramses::SceneConfig(), "basic renderpasses scene");

    // every render pass needs a camera to define rendering parameters
    // create camera with perspective projection
    ramses::PerspectiveCamera* camera = scene->createPerspectiveCamera("perspective camera of renderpass");
    camera->setTranslation(0.0f, 0.0f, 5.0f);
    camera->setFrustum(45.f, 640.f / 480.f, 1.f, 100.f);
    // use left side of the viewport
    camera->setViewport(0u, 0u, 640u, 480u);

    // create render pass
    ramses::RenderPass* renderPass = scene->createRenderPass("renderpass");
    renderPass->setClearFlags(ramses::EClearFlags_None);

    // set valid camera for the pass
    renderPass->setCamera(*camera);

    // create render group and add it to render pass
    ramses::RenderGroup* renderGroup = scene->createRenderGroup("rendergroup");
    renderPass->addRenderGroup(*renderGroup);

    // create another render group and add it to first one as nested render group with render order '3'
    ramses::RenderGroup* nestedRenderGroup= scene->createRenderGroup("nested rendergroup");
    renderGroup->addRenderGroup(*nestedRenderGroup, 3);

    ramses::Appearance* appearanceA = scene->createAppearance(*effectTex, "triangle appearance");
    ramses::Appearance* appearanceB = scene->createAppearance(*effectTex, "triangle appearance");
    ramses::Appearance* appearanceC = scene->createAppearance(*effectTex, "triangle appearance");

    // set vertex positions directly in geometry
    ramses::GeometryBinding* geometry = scene->createGeometryBinding(*effectTex, "triangle geometry");
    geometry->setIndices(*indices);
    ramses::AttributeInput positionsInput;
    effectTex->findAttributeInput("a_position", positionsInput);
    geometry->setInputBuffer(positionsInput, *vertexPositions);

    // use three appearances, each with a different color for distinguishing the meshes
    ramses::UniformInput color;
    effectTex->findUniformInput("color", color);
    appearanceA->setInputValueVector4f(color, 1.0f,0.0f,0.0f,1.0f);
    appearanceB->setInputValueVector4f(color, 0.0f,1.0f,0.0f,1.0f);
    appearanceC->setInputValueVector4f(color, 0.0f,0.0f,1.0f,1.0f);

    ramses::MeshNode* meshNodeA = scene->createMeshNode("red triangle mesh node");
    meshNodeA->setAppearance(*appearanceA);
    meshNodeA->setGeometryBinding(*geometry);

    ramses::MeshNode* meshNodeB = scene->createMeshNode("green triangle mesh node");
    meshNodeB->setAppearance(*appearanceB);
    meshNodeB->setGeometryBinding(*geometry);
    meshNodeB->setTranslation(0.5f, 0.5f, 0.0f);

    ramses::MeshNode* meshNodeC = scene->createMeshNode("blue triangle mesh node");
    meshNodeC->setAppearance(*appearanceC);
    meshNodeC->setGeometryBinding(*geometry);
    meshNodeC->setTranslation(0.75f, -0.25f, 0.0f);

    // Add meshA with render order '1' to the render group where already the nested render group with render order '3' was added.
    // So meshA is rendered before every other element of the nested render group.
    renderGroup->addMeshNode(*meshNodeA, 1);

    // Add meshB and meshC to the nested render group with render order '2' and '1'.
    // This order is only relevant inside the nested render group. So C is rendered before B but both are rendered after A.
    nestedRenderGroup->addMeshNode(*meshNodeB, 2);
    nestedRenderGroup->addMeshNode(*meshNodeC, 1);

    /// [Basic Render Groups Example]

    // signal the scene it is in a state that can be rendered
    scene->flush();

    // distribute the scene to RAMSES
    scene->publish();

    // application logic
    std::this_thread::sleep_for(std::chrono::seconds(10));

    // shutdown: stop distribution, free resources, unregister
    scene->unpublish();
    ramses.destroy(*scene);
    ramses.destroy(*vertexPositions);
    ramses.destroy(*indices);
    framework.disconnect();

    return 0;
}
